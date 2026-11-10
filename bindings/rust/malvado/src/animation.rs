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
        // SAFETY: unique owning pointer; playback borrows prevent early drop.
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
