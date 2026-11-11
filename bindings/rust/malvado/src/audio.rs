// Malvado 2; SPDX-License-Identifier: Apache-2.0
//! Owner-thread audio with explicit playback maintenance and shutdown.
use crate::{Error, window::main_thread};
use malvado_sys::{self as sys, graphics as ffi};
use std::{
    cell::{Cell, RefCell},
    ffi::CString,
    os::unix::ffi::OsStrExt,
    path::Path,
    ptr::NonNull,
    rc::Rc,
    sync::atomic::{AtomicBool, Ordering},
};

static AUDIO_OWNED: AtomicBool = AtomicBool::new(false);

/// One audio device, independent of the window and process scheduler.
/// Keep an Rc<Audio> in the host and call update regularly. Sounds keep the
/// service live; the final service owner stops outstanding playback and closes.
/// Do not open or close the native device through another host while owned.
pub struct Audio {
    device: Rc<Device>,
    sounds: RefCell<Vec<Rc<SoundData>>>,
}
struct Device;
impl Audio {
    pub fn open() -> Result<Self, Error> {
        sys::check_abi().map_err(Error::Abi)?;
        if !main_thread() {
            return Err(Error::Platform(
                "audio requires the application main thread",
            ));
        }
        AUDIO_OWNED
            .compare_exchange(false, true, Ordering::AcqRel, Ordering::Acquire)
            .map_err(|_| Error::Busy)?;
        // SAFETY: the main thread acquired the single safe audio lease.
        if unsafe { ffi::mv_audio_open() } == 0 {
            AUDIO_OWNED.store(false, Ordering::Release);
            return Err(Error::Platform("audio open failed"));
        }
        Ok(Self {
            device: Rc::new(Device),
            sounds: RefCell::new(Vec::new()),
        })
    }
    /// Release completed sound uses. Paused uses stay retained. This does not
    /// advance the engine or poll input. Dropping Audio stops remaining uses.
    pub fn update(&self) {
        self.sounds
            .borrow_mut()
            .retain(|sound| sound.paused.get() || sound.playing());
    }
}
impl Drop for Device {
    fn drop(&mut self) {
        // SAFETY: every native resource has released its device reference.
        unsafe {
            ffi::mv_audio_close();
        }
        AUDIO_OWNED.store(false, Ordering::Release);
    }
}

/// An independent playback instance. Use try_clone to share the decoded WAV
/// data with a new native playback instance, rather than aliasing its controls.
pub struct Sound {
    data: Rc<SoundData>,
    audio: Rc<Audio>,
}
struct SoundData {
    native: NonNull<ffi::MvSound>,
    paused: Cell<bool>,
    _device: Rc<Device>,
}
impl Sound {
    pub fn load(audio: Rc<Audio>, path: impl AsRef<Path>) -> Result<Self, Error> {
        let path = path_string(path)?;
        // SAFETY: live device; native load copies the borrowed path.
        Self::owned(unsafe { ffi::mv_sound_load(path.as_ptr()) }, audio)
    }
    fn owned(native: *mut ffi::MvSound, audio: Rc<Audio>) -> Result<Self, Error> {
        let native = NonNull::new(native).ok_or(Error::Platform("sound load or clone failed"))?;
        Ok(Self {
            data: Rc::new(SoundData {
                native,
                paused: Cell::new(false),
                _device: audio.device.clone(),
            }),
            audio,
        })
    }
    pub fn try_clone(&self) -> Result<Self, Error> {
        // SAFETY: a live native instance; clone returns a new owned instance.
        Self::owned(
            unsafe { ffi::mv_sound_clone(self.data.native.as_ptr()) },
            self.audio.clone(),
        )
    }
    /// Start or restart this instance and retain it until stop/completion.
    pub fn play(&self) -> Result<(), Error> {
        // Reserve retention before changing native playback.
        let mut active = self.audio.sounds.borrow_mut();
        let retained = active.iter().any(|s| Rc::ptr_eq(s, &self.data));
        if !retained {
            active.try_reserve(1).map_err(|_| Error::OutOfMemory)?;
        }
        // SAFETY: live owner-thread sound and device.
        accepted(unsafe { ffi::mv_sound_play(self.data.native.as_ptr()) })?;
        self.data.paused.set(false);
        if !retained {
            active.push(self.data.clone());
        }
        Ok(())
    }
    pub fn stop(&self) -> Result<(), Error> {
        // SAFETY: live owner-thread sound and device.
        accepted(unsafe { ffi::mv_sound_stop(self.data.native.as_ptr()) })?;
        self.data.paused.set(false);
        self.audio
            .sounds
            .borrow_mut()
            .retain(|s| !Rc::ptr_eq(s, &self.data));
        Ok(())
    }
    pub fn pause(&self) -> Result<(), Error> {
        // SAFETY: live owner-thread sound and device.
        accepted(unsafe { ffi::mv_sound_pause(self.data.native.as_ptr()) })?;
        self.data.paused.set(true);
        Ok(())
    }
    pub fn resume(&self) -> Result<(), Error> {
        // SAFETY: live owner-thread sound and device.
        accepted(unsafe { ffi::mv_sound_resume(self.data.native.as_ptr()) })?;
        self.data.paused.set(false);
        Ok(())
    }
    pub fn playing(&self) -> bool {
        self.data.playing()
    }
    /// Set this instance's volume in 0..=1. Invalid values change nothing.
    pub fn set_volume(&self, volume: f32) -> Result<(), Error> {
        volume_valid(volume)?;
        // SAFETY: live owner-thread instance and validated finite volume.
        accepted(unsafe { ffi::mv_sound_set_volume(self.data.native.as_ptr(), volume) })
    }
}
impl SoundData {
    fn playing(&self) -> bool {
        // SAFETY: native instance retains its owner-thread device.
        unsafe { ffi::mv_sound_playing(self.native.as_ptr()) != 0 }
    }
}
impl Drop for SoundData {
    fn drop(&mut self) {
        // SAFETY: last playback/data owner; destroy before releasing the device.
        unsafe {
            ffi::mv_sound_destroy(self.native.as_ptr());
        }
    }
}
fn path_string(path: impl AsRef<Path>) -> Result<CString, Error> {
    CString::new(path.as_ref().as_os_str().as_bytes()).map_err(|_| Error::InvalidArgument)
}
fn volume_valid(volume: f32) -> Result<(), Error> {
    if volume.is_finite() && (0.0..=1.0).contains(&volume) {
        Ok(())
    } else {
        Err(Error::InvalidArgument)
    }
}
fn accepted(result: i32) -> Result<(), Error> {
    if result != 0 {
        Ok(())
    } else {
        Err(Error::Platform("audio operation failed"))
    }
}
/// Audio and its resources stay on the creating thread.
/// ```compile_fail
/// let audio = malvado::audio::Audio::open().unwrap();
/// std::thread::spawn(move || drop(audio));
/// ```
/// ```compile_fail
/// fn invalid(sound: malvado::audio::Sound) {
///     std::thread::spawn(move || drop(sound));
/// }
/// ```
mod ownership_contract {}
