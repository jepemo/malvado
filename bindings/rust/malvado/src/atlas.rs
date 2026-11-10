// Malvado 2; SPDX-License-Identifier: Apache-2.0
//! Owned atlas metadata. This module does not load images or open a window.
use crate::Error;
use malvado_sys::{self as sys, assets as ffi};
use std::{
    ffi::{CStr, CString},
    marker::PhantomData,
    mem::MaybeUninit,
    ptr::NonNull,
    rc::Rc,
};

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Region {
    pub x: i32,
    pub y: i32,
    pub width: i32,
    pub height: i32,
}
/// Geometry is copied. The name borrows the owning atlas.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Sprite<'a> {
    pub id: i32,
    pub name: &'a str,
    pub region: Region,
    pub pivot: (i32, i32),
    pub source_size: (i32, i32),
    pub trim: (i32, i32),
}
/// Immutable native metadata. Parsing copies input; Drop frees it once.
/// Access and destruction stay on the creating thread.
pub struct Atlas {
    native: NonNull<ffi::MvAtlas>,
    local: PhantomData<Rc<()>>,
}
impl Atlas {
    /// Parse Malvado schema 1 or 2 against the actual image dimensions.
    pub fn parse(json: &str, width: i32, height: i32) -> Result<Self, Error> {
        Self::parse_with(json, width, height, ffi::mv_atlas_parse)
    }
    fn parse_with(
        json: &str,
        width: i32,
        height: i32,
        parse: unsafe extern "C" fn(
            *const std::ffi::c_char,
            usize,
            i32,
            i32,
            *mut *const std::ffi::c_char,
        ) -> *mut ffi::MvAtlas,
    ) -> Result<Self, Error> {
        sys::check_abi().map_err(Error::Abi)?;
        let mut error = std::ptr::null();
        // SAFETY: buffer is borrowed for this call; the native result owns a copy.
        let native = unsafe { parse(json.as_ptr().cast(), json.len(), width, height, &mut error) };
        let Some(native) = NonNull::new(native) else {
            let message = if error.is_null() {
                "atlas parse failed"
            } else {
                // SAFETY: the native API returns immutable static error strings.
                let message: &'static CStr = unsafe { CStr::from_ptr(error) };
                message.to_str().unwrap_or("atlas parse failed")
            };
            return Err(Error::Asset(message));
        };
        Ok(Self {
            native,
            local: PhantomData,
        })
    }
    pub fn len(&self) -> usize {
        // SAFETY: live owned atlas.
        unsafe { ffi::mv_atlas_count(self.native.as_ptr()) }
    }
    pub fn is_empty(&self) -> bool {
        self.len() == 0
    }
    pub fn image(&self) -> &str {
        // SAFETY: successful parse owns a non-null terminated UTF-8 image path.
        unsafe { CStr::from_ptr(ffi::mv_atlas_image(self.native.as_ptr())) }
            .to_str()
            .expect("validated atlas image")
    }
    pub fn sprite(&self, id: i32) -> Option<Sprite<'_>> {
        let mut out = MaybeUninit::uninit();
        // SAFETY: native success initializes the complete record.
        if unsafe { ffi::mv_atlas_find(self.native.as_ptr(), id, out.as_mut_ptr()) } == 0 {
            return None;
        }
        // SAFETY: successful find initialized out; name remains owned by self.
        Some(self.sprite_value(unsafe { out.assume_init() }))
    }
    /// Exact, case-sensitive lookup. Embedded NUL is an invalid argument.
    pub fn sprite_named(&self, name: &str) -> Result<Option<Sprite<'_>>, Error> {
        let name = CString::new(name).map_err(|_| Error::InvalidArgument)?;
        let mut out = MaybeUninit::uninit();
        // SAFETY: live input and output, no retained caller strings.
        if unsafe { ffi::mv_atlas_find_name(self.native.as_ptr(), name.as_ptr(), out.as_mut_ptr()) }
            == 0
        {
            return Ok(None);
        }
        // SAFETY: successful find initialized out; name remains owned by self.
        Ok(Some(self.sprite_value(unsafe { out.assume_init() })))
    }
    fn sprite_value(&self, value: ffi::MvAtlasSprite) -> Sprite<'_> {
        // SAFETY: called only with a record borrowed from this live atlas.
        let name = unsafe { CStr::from_ptr(value.name) }
            .to_str()
            .expect("validated sprite name");
        Sprite {
            id: value.id,
            name,
            region: Region {
                x: value.region.x,
                y: value.region.y,
                width: value.region.width,
                height: value.region.height,
            },
            pivot: (value.pivot_x, value.pivot_y),
            source_size: (value.source_width, value.source_height),
            trim: (value.trim_x, value.trim_y),
        }
    }
}
impl Drop for Atlas {
    fn drop(&mut self) {
        // SAFETY: one owned allocation; borrowed views prevent early destruction.
        unsafe {
            ffi::mv_atlas_destroy(self.native.as_ptr());
        }
    }
}
/// Borrowed sprite names cannot outlive their atlas.
/// ```compile_fail
/// use malvado::atlas::Atlas;
/// fn invalid(json: &str) -> &str {
///     let atlas = Atlas::parse(json, 8, 8).unwrap();
///     atlas.sprite(7).unwrap().name
/// }
/// ```
/// Atlas access stays on its creating thread.
/// ```compile_fail
/// use malvado::atlas::Atlas;
/// fn invalid(atlas: Atlas) { std::thread::spawn(move || drop(atlas)); }
/// ```
mod ownership_contract {}
