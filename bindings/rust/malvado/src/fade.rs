// Malvado 2; SPDX-License-Identifier: Apache-2.0
//! A copied native fade queue. The host advances it once per simulation tick.
use crate::{Error, window::Color};
use malvado_sys::{self as sys, graphics as ffi};

/// One timed transition and up to 16 pending requests. Copies are independent.
/// No window or process owns this value; rendering does not advance it.
#[derive(Clone, Copy, Debug)]
pub struct Fade {
    native: ffi::MvFadeQueue,
}
impl Fade {
    /// Start transparent black with an empty queue.
    pub fn new() -> Result<Self, Error> {
        sys::check_abi().map_err(Error::Abi)?;
        Ok(Self {
            native: ffi::MvFadeQueue::default(),
        })
    }
    /// Start immediately if idle, otherwise append. Zero ticks changes color
    /// immediately when started. Duration is 0..=1_000_000 simulation ticks.
    /// Invalid duration or QueueFull leaves all previous work unchanged.
    pub fn enqueue(&mut self, target: Color, ticks: u32) -> Result<(), Error> {
        // SAFETY: private initialized queue, exclusively borrowed for mutation.
        match unsafe { ffi::mv_fade_queue_push(&mut self.native, target, ticks) } {
            1 => Ok(()),
            -1 => Err(Error::QueueFull),
            _ => Err(Error::InvalidArgument),
        }
    }
    /// Advance once. Start queued transitions after completion and drain any
    /// immediate changes on that tick. Return whether queue state changed.
    pub fn step(&mut self) -> bool {
        // SAFETY: private queue preserves all native invariants.
        unsafe { ffi::mv_fade_queue_step(&mut self.native) != 0 }
    }
    pub fn active(&self) -> bool {
        // SAFETY: immutable borrow of initialized native state.
        unsafe { ffi::mv_fade_queue_active(&self.native) != 0 }
    }
    pub fn color(&self) -> Color {
        self.native.fade.current
    }
}

impl crate::window::Frame<'_> {
    /// Draw the current overlay over the whole window, without advancing it.
    /// Call after scene drawing and after clip/scroll scopes have ended.
    /// An active scope returns InvalidArgument without drawing or changing it.
    pub fn fade(&self, fade: &Fade) -> Result<(), Error> {
        // SAFETY: a live owner-thread frame; C rejects active clip/scroll scopes.
        crate::drawing::valid(unsafe { ffi::mv_window_draw_fade(fade.color()) })
    }
}
