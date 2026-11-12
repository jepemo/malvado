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
/// Context-owned persistent text. Clones share identity, and dropping a handle
/// does not remove the entry. Explicit removal, owner cleanup or engine shutdown
/// ends its native lifetime. Snapshots own strings so ticks cannot invalidate them.
/// No implicit font or render registration is included.
#[derive(Clone)]
pub struct TextHandle {
    runtime: Rc<crate::engine::Runtime>,
    id: u64,
}
#[derive(Debug, PartialEq, Eq)]
pub struct OwnedSnapshot {
    pub value: String,
    pub position: (i32, i32),
    pub alignment: TextAlignment,
}
pub(crate) fn register(
    runtime: Rc<crate::engine::Runtime>,
    owner: Option<sys::MvProcessRef>,
    value: &str,
    position: (i32, i32),
    align: TextAlignment,
) -> Result<TextHandle, Error> {
    let value = CString::new(value).map_err(|_| Error::InvalidArgument)?;
    let mut id = 0;
    // SAFETY: live local context and optional identity; C copies content.
    result(unsafe {
        sys::mv_text_entry_create(
            runtime.live()?,
            owner.as_ref().map_or(std::ptr::null(), |r| r),
            value.as_ptr(),
            position.0,
            position.1,
            alignment(align),
            &mut id,
        )
    })?;
    Ok(TextHandle { runtime, id })
}
impl crate::Handle {
    /// Register text owned by this live process. Sleep/freeze retain it.
    pub fn create_text(
        &self,
        value: &str,
        position: (i32, i32),
        align: TextAlignment,
    ) -> Result<TextHandle, Error> {
        register(
            self.inner.runtime.clone(),
            Some(self.inner.reference),
            value,
            position,
            align,
        )
    }
}
impl TextHandle {
    pub fn update(
        &self,
        value: &str,
        position: (i32, i32),
        align: TextAlignment,
    ) -> Result<(), Error> {
        let value = CString::new(value).map_err(|_| Error::InvalidArgument)?;
        // SAFETY: checked local context; the native ID is tied to this runtime.
        result(unsafe {
            sys::mv_text_entry_update(
                self.runtime.live()?,
                self.id,
                value.as_ptr(),
                position.0,
                position.1,
                alignment(align),
            )
        })
    }
    pub fn snapshot(&self) -> Result<OwnedSnapshot, Error> {
        let mut value = std::ptr::null();
        let (mut x, mut y, mut align) = (0, 0, 0);
        // SAFETY: distinct outputs, live owner-thread context and private ID.
        result(unsafe {
            sys::mv_text_entry_read(
                self.runtime.live()?,
                self.id,
                &mut value,
                &mut x,
                &mut y,
                &mut align,
            )
        })?;
        // SAFETY: native borrowed string remains live during this synchronous
        // copy. No scheduler calls or user callbacks run between read and copy.
        let value = unsafe { CStr::from_ptr(value) }
            .to_string_lossy()
            .into_owned();
        Ok(OwnedSnapshot {
            value,
            position: (x, y),
            alignment: match align {
                0 => TextAlignment::Left,
                1 => TextAlignment::Center,
                2 => TextAlignment::Right,
                _ => unreachable!("native alignment"),
            },
        })
    }
    /// Remove once. All clones then return NotFound until engine shutdown.
    pub fn remove(&self) -> Result<(), Error> {
        // SAFETY: private context-local identity, validated by the core.
        result(unsafe { sys::mv_text_entry_remove(self.runtime.live()?, self.id) })
    }
}

#[cfg(feature = "graphics")]
impl TextHandle {
    /// Retain a custom font and its window until style replacement, clear,
    /// entry removal, owner cleanup or shutdown. Does not draw implicitly.
    pub fn set_font(
        &self,
        font: &crate::font::Font,
        size: i32,
        color: crate::window::Color,
    ) -> Result<(), Error> {
        self.bind_font(font.native(), size, color, font.clone())
    }
    /// Retain the window while this entry uses the built-in font.
    pub fn set_builtin_font(
        &self,
        window: Rc<crate::window::Window>,
        size: i32,
        color: crate::window::Color,
    ) -> Result<(), Error> {
        self.bind_font(std::ptr::null_mut(), size, color, window)
    }
    fn bind_font<T>(
        &self,
        font: *mut sys::graphics::MvFont,
        size: i32,
        color: crate::window::Color,
        owner: T,
    ) -> Result<(), Error> {
        let ctx = self.runtime.live()?;
        let owner = Box::into_raw(Box::new(owner));
        // SAFETY: native acquires the font and takes the boxed lifetime owner
        // only on success. Font or window keeps the main-thread device alive.
        let status = unsafe {
            sys::graphics::mv_text_entry_font_owned(
                ctx,
                self.id,
                font,
                size,
                color,
                owner.cast(),
                Some(release_text_owner::<T>),
            )
        };
        if let Err(error) = result(status) {
            // SAFETY: failure leaves the box with this caller.
            unsafe {
                drop(Box::from_raw(owner));
            }
            return Err(error);
        }
        Ok(())
    }
    pub fn clear_font(&self) -> Result<(), Error> {
        // SAFETY: live local context and private ID; native releases owners.
        result(unsafe { sys::graphics::mv_text_entry_font_clear(self.runtime.live()?, self.id) })
    }
}
#[cfg(feature = "graphics")]
unsafe extern "C" fn release_text_owner<T>(owner: *mut std::ffi::c_void) {
    // SAFETY: each successful bind transfers exactly one Box; no unwind into C.
    let _ = crate::task::contain(|| unsafe {
        drop(Box::from_raw(owner.cast::<T>()));
    });
}
#[cfg(feature = "graphics")]
impl crate::window::Frame<'_> {
    /// Draw native retained text after process graphics and before an overlay.
    /// Active clip/scroll scopes return an error without drawing.
    pub fn texts(&self, engine: &crate::Engine) -> Result<(), Error> {
        // SAFETY: frame-bound drawing and a live owner-thread engine.
        crate::drawing::valid(unsafe {
            sys::graphics::mv_window_draw_texts(engine.native_context()?)
        })
    }
}
