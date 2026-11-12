// Malvado 2; SPDX-License-Identifier: Apache-2.0
//! Owned native fonts. Frames retain queued text until drawing completes.
use crate::{
    Error,
    drawing::valid,
    window::{Color, Frame, Window},
};
use malvado_sys::graphics as ffi;
use std::{ffi::CString, os::unix::ffi::OsStrExt, path::Path, ptr::NonNull, rc::Rc};

/// Clone shares an immutable font and its retained window. Each native load is
/// independent; there is no additional font cache in the Rust binding.
#[derive(Clone)]
pub struct Font {
    inner: Rc<Inner>,
}
struct Inner {
    native: NonNull<ffi::MvFont>,
    window: Rc<Window>,
}
impl Font {
    pub(crate) fn native(&self) -> *mut ffi::MvFont {
        self.inner.native.as_ptr()
    }

    /// Load a TTF/OTF with the native ASCII and Latin-1 glyph set.
    pub fn load(window: Rc<Window>, path: impl AsRef<Path>, base_size: i32) -> Result<Self, Error> {
        if !(1..=256).contains(&base_size) {
            return Err(Error::InvalidArgument);
        }
        let path = CString::new(path.as_ref().as_os_str().as_bytes())
            .map_err(|_| Error::InvalidArgument)?;
        // SAFETY: retained live window and borrowed terminated native path.
        Self::owned(
            unsafe { ffi::mv_font_load(path.as_ptr(), base_size) },
            window,
        )
    }
    /// Load 1..1024 distinct Unicode scalar glyphs. NUL is unsupported.
    pub fn load_glyphs(
        window: Rc<Window>,
        path: impl AsRef<Path>,
        base_size: i32,
        glyphs: &[char],
    ) -> Result<Self, Error> {
        if !(1..=256).contains(&base_size)
            || glyphs.is_empty()
            || glyphs.len() > 1024
            || glyphs
                .iter()
                .enumerate()
                .any(|(index, &glyph)| glyph == '\0' || glyphs[..index].contains(&glyph))
        {
            return Err(Error::InvalidArgument);
        }
        let path = CString::new(path.as_ref().as_os_str().as_bytes())
            .map_err(|_| Error::InvalidArgument)?;
        let codepoints: Vec<u32> = glyphs.iter().map(|&glyph| u32::from(glyph)).collect();
        // SAFETY: native load borrows validated scalar data for this call only.
        Self::owned(
            unsafe {
                ffi::mv_font_load_codepoints(
                    path.as_ptr(),
                    base_size,
                    codepoints.as_ptr(),
                    codepoints.len(),
                )
            },
            window,
        )
    }
    fn owned(native: *mut ffi::MvFont, window: Rc<Window>) -> Result<Self, Error> {
        Ok(Self {
            inner: Rc::new(Inner {
                native: NonNull::new(native).ok_or(Error::Platform("font load failed"))?,
                window,
            }),
        })
    }
    /// Zero means empty text or a native unrepresentable width. Size is 1..1024.
    pub fn text_width(&self, text: &str, size: i32) -> Result<i32, Error> {
        if !(1..=1024).contains(&size) {
            return Err(Error::InvalidArgument);
        }
        let text = CString::new(text).map_err(|_| Error::InvalidArgument)?;
        // SAFETY: immutable font keeps its native window open for measurement.
        Ok(unsafe { ffi::mv_font_text_width(self.inner.native.as_ptr(), text.as_ptr(), size) })
    }
}
impl Drop for Inner {
    fn drop(&mut self) {
        // SAFETY: release one native owner before dropping its retained window.
        unsafe {
            ffi::mv_font_destroy(self.native.as_ptr());
        }
    }
}
impl Frame<'_> {
    pub fn font_text(
        &self,
        font: &Font,
        text: &str,
        position: (i32, i32),
        size: i32,
        color: Color,
    ) -> Result<(), Error> {
        if !std::ptr::eq(self.window, font.inner.window.as_ref()) {
            return Err(Error::ForeignContext);
        }
        if !(1..=1024).contains(&size) {
            return Err(Error::InvalidArgument);
        }
        let text = CString::new(text).map_err(|_| Error::InvalidArgument)?;
        self.fonts.borrow_mut().push(font.clone());
        // SAFETY: queued text retains its font until after native EndDrawing.
        valid(unsafe {
            ffi::mv_font_draw(
                font.inner.native.as_ptr(),
                text.as_ptr(),
                position.0,
                position.1,
                size,
                color,
            )
        })
    }
}
/// Font data and native cleanup stay on the window owner thread.
/// ```compile_fail
/// use malvado::font::Font;
/// fn invalid(font: Font) { std::thread::spawn(move || drop(font)); }
/// ```
mod ownership_contract {}
