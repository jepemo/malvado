// Malvado 2; SPDX-License-Identifier: Apache-2.0
//! Owned native text content. Font and rendering registration remain explicit.
use crate::{Error, engine::result, graphic::TextAlignment};
use malvado_sys as sys;
use std::{
    ffi::{CStr, CString},
    marker::PhantomData,
    ptr::NonNull,
    rc::Rc,
};

/// Mutable text storage, independent of a window or process.
/// Borrowed snapshots prevent update or destruction while in use.
/// ```compile_fail
/// use malvado::{text::Text, graphic::TextAlignment};
/// let mut text = Text::new("old", (0,0), TextAlignment::Left).unwrap();
/// let snapshot = text.snapshot();
/// text.update("new", (1,2), TextAlignment::Right).unwrap();
/// println!("{}", snapshot.value);
/// ```
/// ```compile_fail
/// use malvado::{text::Text, graphic::TextAlignment};
/// let text = Text::new("local", (0,0), TextAlignment::Left).unwrap();
/// std::thread::spawn(move || drop(text));
/// ```
pub struct Text {
    native: NonNull<sys::MvText>,
    local: PhantomData<Rc<()>>,
}
#[derive(Debug, PartialEq, Eq)]
pub struct Snapshot<'a> {
    pub value: &'a str,
    pub position: (i32, i32),
    pub alignment: TextAlignment,
}
fn alignment(value: TextAlignment) -> i32 {
    match value {
        TextAlignment::Left => 0,
        TextAlignment::Center => 1,
        TextAlignment::Right => 2,
    }
}
impl Text {
    /// Copies UTF-8 content. Embedded NUL returns InvalidArgument.
    /// ```
    /// use malvado::{text::Text, graphic::TextAlignment};
    /// let mut text = Text::new("Score: 0", (10, 10), TextAlignment::Left)?;
    /// text.update("Score: 1", (10, 10), TextAlignment::Left)?;
    /// assert_eq!(text.snapshot().value, "Score: 1");
    /// # Ok::<(), malvado::Error>(())
    /// ```
    pub fn new(value: &str, position: (i32, i32), align: TextAlignment) -> Result<Self, Error> {
        sys::check_abi().map_err(Error::Abi)?;
        let value = CString::new(value).map_err(|_| Error::InvalidArgument)?;
        let mut native = std::ptr::null_mut();
        // SAFETY: live string and exclusive output; native copies the string.
        result(unsafe {
            sys::mv_text_create(
                value.as_ptr(),
                position.0,
                position.1,
                alignment(align),
                &mut native,
            )
        })?;
        Ok(Self {
            native: NonNull::new(native).ok_or(Error::OutOfMemory)?,
            local: PhantomData,
        })
    }
    /// Replace content and position atomically. Alignment is explicit.
    pub fn update(
        &mut self,
        value: &str,
        position: (i32, i32),
        align: TextAlignment,
    ) -> Result<(), Error> {
        let value = CString::new(value).map_err(|_| Error::InvalidArgument)?;
        // SAFETY: exclusive owner and live copied string.
        result(unsafe {
            sys::mv_text_update(
                self.native.as_ptr(),
                value.as_ptr(),
                position.0,
                position.1,
                alignment(align),
            )
        })
    }
    pub fn snapshot(&self) -> Snapshot<'_> {
        let mut value = std::ptr::null();
        let (mut x, mut y, mut align) = (0, 0, 0);
        // SAFETY: live owner and distinct initialized outputs. The native string
        // remains valid under this shared borrow; safe mutation requires &mut.
        let status = unsafe {
            sys::mv_text_read(self.native.as_ptr(), &mut value, &mut x, &mut y, &mut align)
        };
        assert_eq!(status, sys::MV_OK);
        // SAFETY: native owns a terminated copy of the UTF-8 input.
        let value = unsafe { CStr::from_ptr(value) }
            .to_str()
            .expect("native UTF-8 copy");
        Snapshot {
            value,
            position: (x, y),
            alignment: match align {
                0 => TextAlignment::Left,
                1 => TextAlignment::Center,
                2 => TextAlignment::Right,
                _ => unreachable!("native alignment"),
            },
        }
    }
}
impl Drop for Text {
    fn drop(&mut self) {
        // SAFETY: unique owner released once on its creating thread.
        unsafe { sys::mv_text_destroy(self.native.as_ptr()) }
    }
}
#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn copied_content_and_atomic_updates() {
        let input = String::from("Hola 🦀");
        let mut text = Text::new(&input, (i32::MIN, i32::MAX), TextAlignment::Center).unwrap();
        drop(input);
        assert_eq!(text.snapshot().value, "Hola 🦀");
        assert!(matches!(
            text.update("bad\0value", (0, 0), TextAlignment::Left),
            Err(Error::InvalidArgument)
        ));
        assert_eq!(text.snapshot().position, (i32::MIN, i32::MAX));
        assert_eq!(text.snapshot().alignment, TextAlignment::Center);
        text.update("", (1, 2), TextAlignment::Right).unwrap();
        assert_eq!(
            text.snapshot(),
            Snapshot {
                value: "",
                position: (1, 2),
                alignment: TextAlignment::Right
            }
        );
        assert!(matches!(
            Text::new("\0", (0, 0), TextAlignment::Left),
            Err(Error::InvalidArgument)
        ));
    }
}
