// Malvado 2; SPDX-License-Identifier: Apache-2.0
//! Retained process commands. Native storage owns order, copied text and images.
use crate::{Error, Handle, engine::result, graphic::Color};
use malvado_sys as sys;
use std::ffi::CString;

impl Handle {
    /// Append a world-space box. Graph transforms do not affect commands.
    pub fn draw_box(
        &self,
        position: (i32, i32),
        size: (i32, i32),
        filled: bool,
        color: Color,
    ) -> Result<(), Error> {
        // SAFETY: live owner-thread context and copied values validated by C.
        result(unsafe {
            sys::mv_process_draw_box(
                self.inner.runtime.live()?,
                self.inner.reference,
                position.0,
                position.1,
                size.0,
                size.1,
                filled.into(),
                color,
            )
        })
    }
    pub fn draw_circle(
        &self,
        center: (i32, i32),
        radius: i32,
        filled: bool,
        color: Color,
    ) -> Result<(), Error> {
        // SAFETY: live identity and copied values; C checks integer boundaries.
        result(unsafe {
            sys::mv_process_draw_circle(
                self.inner.runtime.live()?,
                self.inner.reference,
                center.0,
                center.1,
                radius,
                filled.into(),
                color,
            )
        })
    }
    /// Copy text into the native list. Embedded NUL bytes are rejected.
    pub fn draw_text(
        &self,
        text: &str,
        position: (i32, i32),
        size: i32,
        color: Color,
    ) -> Result<(), Error> {
        let text = CString::new(text).map_err(|_| Error::InvalidArgument)?;
        // SAFETY: C copies the borrowed C string before return.
        result(unsafe {
            sys::mv_process_draw_text(
                self.inner.runtime.live()?,
                self.inner.reference,
                text.as_ptr(),
                position.0,
                position.1,
                size,
                color,
            )
        })
    }
    pub fn draw_count(&self) -> Result<usize, Error> {
        let mut count = 0;
        // SAFETY: distinct output; C validates the context-bound identity.
        result(unsafe {
            sys::mv_process_draw_count(self.inner.runtime.live()?, self.inner.reference, &mut count)
        })?;
        Ok(count)
    }
    /// Release commands now. Actual future polls also clear the previous list.
    pub fn clear_draw_commands(&self) -> Result<(), Error> {
        // SAFETY: owner-thread context; native cleanup releases each owner once.
        result(unsafe {
            sys::mv_process_draw_clear(self.inner.runtime.live()?, self.inner.reference)
        })
    }
}
