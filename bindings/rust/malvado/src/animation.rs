// Malvado 2; SPDX-License-Identifier: Apache-2.0
//! Immutable native animation definitions with explicit fixed-tick playback.
use crate::Error;
use malvado_sys as sys;
use std::{marker::PhantomData, ptr::NonNull, rc::Rc};

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Frame {
    pub graph_id: i32,
    pub duration_us: u64,
}
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Sample {
    pub index: usize,
    pub graph_id: i32,
}
/// Owns the native copied frame definition. Graph IDs do not retain assets.
/// Destruction stays on the creating thread.
pub struct Animation {
    native: NonNull<sys::MvAnimation>,
    local: PhantomData<Rc<()>>,
}
impl Animation {
    /// Number of immutable frames. Valid definitions are never empty.
    pub fn len(&self) -> usize {
        // SAFETY: this owner retains the live immutable native definition.
        unsafe { sys::mv_animation_count(self.native.as_ptr()) }
    }
    pub fn is_empty(&self) -> bool {
        self.len() == 0
    }
    /// Accept 1..8192 frames, positive graph IDs, and durations 1..3600000000 us.
    pub fn new(frames: &[Frame], looping: bool) -> Result<Self, Error> {
        sys::check_abi().map_err(Error::Abi)?;
        if frames.is_empty()
            || frames.len() > 8192
            || frames
                .iter()
                .any(|f| f.graph_id <= 0 || !(1..=3_600_000_000).contains(&f.duration_us))
        {
            return Err(Error::InvalidArgument);
        }
        let frames: Vec<_> = frames
            .iter()
            .map(|f| sys::MvAnimationFrame {
                graph_id: f.graph_id,
                duration_us: f.duration_us,
            })
            .collect();
        // SAFETY: native create copies this validated contiguous frame slice.
        let native =
            unsafe { sys::mv_animation_create(frames.as_ptr(), frames.len(), looping.into()) };
        Ok(Self {
            native: NonNull::new(native).ok_or(Error::OutOfMemory)?,
            local: PhantomData,
        })
    }
    /// Loop or hold the last frame using the native sequence rules.
    pub fn sample(&self, elapsed_us: u64) -> Sample {
        let mut out = sys::MvAnimationSample::default();
        // SAFETY: owned immutable definition and exclusive output are live.
        let valid = unsafe { sys::mv_animation_sample(self.native.as_ptr(), elapsed_us, &mut out) };
        assert_eq!(valid, 1, "live animation sample");
        Sample {
            index: out.index,
            graph_id: out.graph_id,
        }
    }
    pub fn playback(&self) -> Playback<'_> {
        Playback {
            animation: self,
            state: sys::MvAnimationState::default(),
        }
    }
}
impl Drop for Animation {
    fn drop(&mut self) {
        // SAFETY: release this owned reference; process clocks retain their own.
        unsafe {
            sys::mv_animation_destroy(self.native.as_ptr());
        }
    }
}
/// Independent playback tied to its borrowed definition. No implicit ticks.
pub struct Playback<'a> {
    animation: &'a Animation,
    state: sys::MvAnimationState,
}
impl Playback<'_> {
    pub fn sample(&self) -> Sample {
        self.animation.sample(self.state.elapsed_us)
    }
    pub fn elapsed_us(&self) -> u64 {
        self.state.elapsed_us
    }
    /// Advance once at 1..1000000 ticks/second. Reset before changing rate.
    /// Invalid rate or a rate change leaves the state unchanged.
    pub fn advance(&mut self, tick_rate: u32) -> Result<Sample, Error> {
        // SAFETY: definition is borrowed for this playback's entire lifetime;
        // state is exclusive and native failure preserves it.
        let valid = unsafe {
            sys::mv_animation_advance(self.animation.native.as_ptr(), &mut self.state, tick_rate)
        };
        if valid != 1 {
            return Err(Error::InvalidArgument);
        }
        Ok(self.sample())
    }
    pub fn reset(&mut self) {
        self.state = sys::MvAnimationState::default();
    }
}
/// Playback cannot outlive the definition whose native pointer it borrows.
/// ```compile_fail
/// use malvado::animation::{Animation, Frame};
/// let playback = {
///     let animation = Animation::new(&[Frame { graph_id: 1, duration_us: 1000 }], true).unwrap();
///     animation.playback()
/// };
/// playback.sample();
/// ```
/// Owned definitions remain on their creating thread.
/// ```compile_fail
/// use malvado::animation::{Animation, Frame};
/// let animation = Animation::new(&[Frame { graph_id: 1, duration_us: 1000 }], true).unwrap();
/// std::thread::spawn(move || drop(animation));
/// ```
mod ownership_contract {}

/// A copied process clock snapshot. Graph IDs remain metadata, not assets.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct ProcessClock {
    pub elapsed_us: u64,
    pub sample: Sample,
}
impl crate::Handle {
    /// Retain this definition in the native process and reset its clock.
    /// Uses Game's fixed 60 Hz logical ticks, independently of process FPS.
    /// No image selection or drawing is implicit.
    pub fn set_animation_clock(&self, animation: &Animation) -> Result<(), Error> {
        // SAFETY: live owner-thread context and borrowed immutable definition;
        // native success retains its own reference before this borrow ends.
        crate::engine::result(unsafe {
            sys::mv_process_animation_clock_set(
                self.inner.runtime.live()?,
                self.inner.reference,
                animation.native.as_ptr(),
                60,
            )
        })
    }
    pub fn animation_clock(&self) -> Result<Option<ProcessClock>, Error> {
        let mut state = std::mem::MaybeUninit::uninit();
        let mut sample = std::mem::MaybeUninit::uninit();
        let mut present = 0;
        // SAFETY: distinct output storage. Native initializes both records only
        // on success with presence set; no borrowed native pointer escapes.
        crate::engine::result(unsafe {
            sys::mv_process_animation_clock_get(
                self.inner.runtime.live()?,
                self.inner.reference,
                state.as_mut_ptr(),
                sample.as_mut_ptr(),
                &mut present,
            )
        })?;
        if present == 0 {
            return Ok(None);
        }
        // SAFETY: successful presence initialized complete copied records.
        let (state, sample) = unsafe { (state.assume_init(), sample.assume_init()) };
        Ok(Some(ProcessClock {
            elapsed_us: state.elapsed_us,
            sample: Sample {
                index: sample.index,
                graph_id: sample.graph_id,
            },
        }))
    }
    /// Stop this process clock. Does not change the process graphic.
    pub fn clear_animation_clock(&self) -> Result<(), Error> {
        // SAFETY: native identity and live owner-thread context.
        crate::engine::result(unsafe {
            sys::mv_process_animation_clock_clear(self.inner.runtime.live()?, self.inner.reference)
        })
    }
}
