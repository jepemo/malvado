// Malvado 2; SPDX-License-Identifier: Apache-2.0
//! Retained process commands. Native storage owns order, copied text and images.
use crate::{
    Error, Handle,
    engine::result,
    graphic::{Color, TextAlignment},
};
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
        self.draw_text_aligned(text, position, size, color, TextAlignment::Left)
    }

    /// Append built-in text anchored horizontally at `position.0`.
    pub fn draw_text_aligned(
        &self,
        text: &str,
        position: (i32, i32),
        size: i32,
        color: Color,
        alignment: TextAlignment,
    ) -> Result<(), Error> {
        let text = CString::new(text).map_err(|_| Error::InvalidArgument)?;
        // SAFETY: C copies the borrowed C string before return.
        result(unsafe {
            sys::mv_process_draw_text_aligned(
                self.inner.runtime.live()?,
                self.inner.reference,
                text.as_ptr(),
                position.0,
                position.1,
                size,
                alignment.native(),
                color,
            )
        })
    }

    /// Append custom font text in size 1..=1024. The command keeps the font
    /// and its window alive through sleep or freeze until the queue is cleared.
    #[cfg(feature = "graphics")]
    pub fn draw_text_with_font(
        &self,
        font: &crate::font::Font,
        text: &str,
        position: (i32, i32),
        size: i32,
        color: Color,
        alignment: TextAlignment,
    ) -> Result<(), Error> {
        let text = CString::new(text).map_err(|_| Error::InvalidArgument)?;
        let ctx = self.inner.runtime.live()?;
        let owner = Box::into_raw(Box::new(font.clone()));
        // SAFETY: C retains the font and takes the boxed owner on success.
        let status = unsafe {
            sys::graphics::mv_process_draw_text_font_owned(
                ctx,
                self.inner.reference,
                text.as_ptr(),
                position.0,
                position.1,
                size,
                alignment.native(),
                color,
                font.native(),
                owner.cast(),
                Some(crate::text::release_text_owner::<crate::font::Font>),
            )
        };
        if let Err(error) = result(status) {
            // SAFETY: on failure C leaves ownership with this caller.
            unsafe { drop(Box::from_raw(owner)) };
            return Err(error);
        }
        Ok(())
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

impl crate::ProcessContext {
    pub fn draw_text(
        &self,
        text: &str,
        position: (i32, i32),
        size: i32,
        color: Color,
    ) -> Result<(), Error> {
        self.current.draw_text(text, position, size, color)
    }

    pub fn draw_text_aligned(
        &self,
        text: &str,
        position: (i32, i32),
        size: i32,
        color: Color,
        alignment: TextAlignment,
    ) -> Result<(), Error> {
        self.current
            .draw_text_aligned(text, position, size, color, alignment)
    }

    #[cfg(feature = "graphics")]
    pub fn draw_text_with_font(
        &self,
        font: &crate::font::Font,
        text: &str,
        position: (i32, i32),
        size: i32,
        color: Color,
        alignment: TextAlignment,
    ) -> Result<(), Error> {
        self.current
            .draw_text_with_font(font, text, position, size, color, alignment)
    }

    pub fn draw_box(
        &self,
        position: (i32, i32),
        size: (i32, i32),
        filled: bool,
        color: Color,
    ) -> Result<(), Error> {
        self.current.draw_box(position, size, filled, color)
    }

    pub fn draw_circle(
        &self,
        center: (i32, i32),
        radius: i32,
        filled: bool,
        color: Color,
    ) -> Result<(), Error> {
        self.current.draw_circle(center, radius, filled, color)
    }
}
