// Malvado 2; SPDX-License-Identifier: Apache-2.0
//! Immediate native drawing. Scoped state restores on return or unwind.
use crate::{
    Error,
    window::{Color, Frame, Window},
};
use malvado_sys::graphics as ffi;
use std::ffi::CString;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Rectangle {
    pub x: i32,
    pub y: i32,
    pub width: i32,
    pub height: i32,
}
impl Frame<'_> {
    pub fn rectangle(&self, area: Rectangle, filled: bool, color: Color) -> Result<(), Error> {
        // SAFETY: the frame keeps native drawing active; C validates geometry.
        valid(unsafe {
            ffi::mv_window_draw_box(
                area.x,
                area.y,
                area.width,
                area.height,
                filled.into(),
                color,
            )
        })
    }
    pub fn circle(
        &self,
        center: (i32, i32),
        radius: i32,
        filled: bool,
        color: Color,
    ) -> Result<(), Error> {
        // SAFETY: active frame; native validation includes integer boundaries.
        valid(unsafe {
            ffi::mv_window_draw_circle(center.0, center.1, radius, filled.into(), color)
        })
    }
    pub fn transformed_rectangle(
        &self,
        position: (f32, f32),
        size: (f32, f32),
        pivot: (f32, f32),
        angle: f32,
        color: Color,
    ) -> Result<(), Error> {
        // SAFETY: values only, with native finite/positive geometry checks.
        valid(unsafe {
            ffi::mv_window_draw_transformed_rect(
                position.0, position.1, size.0, size.1, pivot.0, pivot.1, angle, color,
            )
        })
    }
    pub fn filled_circle(
        &self,
        center: (f32, f32),
        radius: f32,
        color: Color,
    ) -> Result<(), Error> {
        // SAFETY: active frame and copied values; C validates geometry.
        valid(unsafe { ffi::mv_window_draw_filled_circle(center.0, center.1, radius, color) })
    }
    pub fn text(
        &self,
        text: &str,
        position: (i32, i32),
        size: i32,
        color: Color,
    ) -> Result<(), Error> {
        if size < 1 {
            return Err(Error::InvalidArgument);
        }
        let text = CString::new(text).map_err(|_| Error::InvalidArgument)?;
        // SAFETY: live frame and terminated UTF-8 text, borrowed only for the call.
        unsafe {
            ffi::mv_window_draw_text(text.as_ptr(), position.0, position.1, size, color);
        }
        Ok(())
    }
    /// Intersect drawing with an in-window rectangle. Nested clips fail.
    pub fn with_clip<T>(
        &self,
        area: Rectangle,
        draw: impl FnOnce(&Self) -> Result<T, Error>,
    ) -> Result<T, Error> {
        // SAFETY: active borrowed frame; native failure preserves previous state.
        valid(unsafe { ffi::mv_window_clip_begin(area.x, area.y, area.width, area.height) })?;
        let _restore = Restore::Clip;
        draw(self)
    }
    /// Translate drawing by (-x,-y). Start a clip before entering this scope.
    /// Nested scrolls and clip changes inside a scroll fail without mutation.
    pub fn with_scroll<T>(
        &self,
        offset: (f32, f32),
        draw: impl FnOnce(&Self) -> Result<T, Error>,
    ) -> Result<T, Error> {
        // SAFETY: active frame; C validates the transform and scope nesting.
        valid(unsafe { ffi::mv_window_scroll_begin(offset.0, offset.1) })?;
        let _restore = Restore::Scroll;
        draw(self)
    }
}
impl Window {
    /// Built-in font width. Native zero covers empty text and unrepresentable width.
    pub fn text_width(&self, text: &str, size: i32) -> Result<i32, Error> {
        if size < 1 {
            return Err(Error::InvalidArgument);
        }
        let text = CString::new(text).map_err(|_| Error::InvalidArgument)?;
        // SAFETY: live owner-thread window and terminated borrowed string.
        Ok(unsafe { ffi::mv_window_text_width(text.as_ptr(), size) })
    }
}
pub(crate) fn valid(value: i32) -> Result<(), Error> {
    if value == 1 {
        Ok(())
    } else {
        Err(Error::InvalidArgument)
    }
}
enum Restore {
    Clip,
    Scroll,
}
impl Drop for Restore {
    fn drop(&mut self) {
        // SAFETY: scopes cannot outlive the borrowed frame. Restoration also
        // runs during unwind; no callback or Rust panic crosses C.
        unsafe {
            match self {
                Self::Clip => {
                    ffi::mv_window_clip_end();
                }
                Self::Scroll => {
                    ffi::mv_window_scroll_end();
                }
            }
        }
    }
}

/// Copied native drawing defaults. Use its colors as explicit frame/draw
/// arguments. It does not change any window, frame, or process implicitly.
#[derive(Clone, Copy, Debug)]
pub struct DrawState {
    native: ffi::MvDrawState,
}
impl DrawState {
    pub fn new() -> Result<Self, Error> {
        malvado_sys::check_abi().map_err(Error::Abi)?;
        let mut native = ffi::MvDrawState::default();
        // SAFETY: exclusively borrowed stack output; native retains nothing.
        valid(unsafe { ffi::mv_draw_state_init(&mut native) })?;
        Ok(Self { native })
    }
    pub fn color(&self) -> Color {
        self.native.color
    }
    pub fn background(&self) -> Color {
        self.native.background
    }
    pub fn set_color(&mut self, color: Color) {
        // SAFETY: exclusive state and channels bounded by their u8 type.
        unsafe {
            ffi::mv_draw_state_color(
                &mut self.native,
                color.r.into(),
                color.g.into(),
                color.b.into(),
                color.a.into(),
            );
        }
    }
    pub fn set_background(&mut self, rgb: (u8, u8, u8)) {
        // SAFETY: exclusive state and typed channels; background stays opaque.
        unsafe {
            ffi::mv_draw_state_background(
                &mut self.native,
                rgb.0.into(),
                rgb.1.into(),
                rgb.2.into(),
            );
        }
    }
}
