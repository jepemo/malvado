// Malvado 2; SPDX-License-Identifier: Apache-2.0
//! Native uncompressed BennuGD 8-bit FPG data. Other variants remain unsupported.
use crate::Error;
use malvado_sys::{self as sys, assets as ffi};
use std::{
    ffi::{CStr, c_char},
    marker::PhantomData,
    mem::MaybeUninit,
    ptr::NonNull,
    rc::Rc,
};

/// Fixed byte fields preserve the source encoding; they need not end with NUL.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct SpriteInfo {
    pub id: i32,
    pub width: i32,
    pub height: i32,
    pub name: [u8; 32],
    pub filename: [u8; 12],
    pub point_count: usize,
}
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Point {
    pub x: i32,
    pub y: i32,
    pub defined: bool,
}
/// Pixel indexes borrow the immutable owning library; metadata is copied.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Sprite<'a> {
    pub info: SpriteInfo,
    pub pixels: &'a [u8],
}
/// Owns copied native data and releases it once on its creating thread.
pub struct Fpg {
    native: NonNull<ffi::MvFpg>,
    local: PhantomData<Rc<()>>,
}
impl Fpg {
    /// Take one uniquely owned, fully validated native allocation.
    /// SAFETY: the pointer must come from the matching native importer and
    /// must have no other owning wrapper or pending destruction.
    pub(crate) unsafe fn from_native(native: NonNull<ffi::MvFpg>) -> Self {
        Self {
            native,
            local: PhantomData,
        }
    }

    pub fn parse(data: &[u8]) -> Result<Self, Error> {
        sys::check_abi().map_err(Error::Abi)?;
        let mut error = std::ptr::null();
        // SAFETY: native parsing borrows this buffer only during the call and
        // copies all accepted data. Error strings have static storage.
        let native = unsafe { ffi::mv_fpg_parse(data.as_ptr().cast(), data.len(), &mut error) };
        Ok(Self {
            native: NonNull::new(native).ok_or_else(|| parse_error(error))?,
            local: PhantomData,
        })
    }
    pub fn len(&self) -> usize {
        // SAFETY: live owned library.
        unsafe { ffi::mv_fpg_count(self.native.as_ptr()) }
    }
    pub fn is_empty(&self) -> bool {
        self.len() == 0
    }
    /// Raw six-bit RGB channels. Conversion policy is separate.
    pub fn palette(&self) -> &[u8; 768] {
        // SAFETY: a live FPG always owns the complete fixed-size palette block.
        unsafe { &*ffi::mv_fpg_palette(self.native.as_ptr()).cast::<[u8; 768]>() }
    }
    pub fn gamma(&self) -> &[u8; 576] {
        // SAFETY: a live FPG always owns the complete fixed-size gamma block.
        unsafe { &*ffi::mv_fpg_gamma(self.native.as_ptr()).cast::<[u8; 576]>() }
    }
    pub fn sprite(&self, id: i32) -> Option<Sprite<'_>> {
        let mut out = MaybeUninit::uninit();
        // SAFETY: output is initialized only on native lookup success.
        if unsafe { ffi::mv_fpg_sprite(self.native.as_ptr(), id, out.as_mut_ptr()) } == 0 {
            return None;
        }
        // SAFETY: successful lookup initialized the complete record.
        Some(self.sprite_value(unsafe { out.assume_init() }))
    }
    /// Zero-based enumeration in original file order.
    pub fn at(&self, index: usize) -> Option<Sprite<'_>> {
        let mut out = MaybeUninit::uninit();
        // SAFETY: native enumeration validates the index before writing output.
        if unsafe { ffi::mv_fpg_at(self.native.as_ptr(), index, out.as_mut_ptr()) } == 0 {
            return None;
        }
        // SAFETY: successful lookup initialized the complete record.
        Some(self.sprite_value(unsafe { out.assume_init() }))
    }
    pub fn point(&self, id: i32, index: usize) -> Option<Point> {
        let mut out = MaybeUninit::uninit();
        // SAFETY: output is initialized only on native lookup success.
        if unsafe { ffi::mv_fpg_point(self.native.as_ptr(), id, index, out.as_mut_ptr()) } == 0 {
            return None;
        }
        // SAFETY: successful lookup initialized all fields.
        Some(point_value(unsafe { out.assume_init() }))
    }
    fn sprite_value(&self, value: ffi::MvFpgSprite) -> Sprite<'_> {
        let info = SpriteInfo {
            id: value.id,
            width: value.width,
            height: value.height,
            name: value.name,
            filename: value.filename,
            point_count: value.point_count,
        };
        // SAFETY: native validation guarantees positive bounded dimensions and
        // this many contiguous indexes owned by self. Borrow cannot outlive self.
        let pixels = unsafe {
            std::slice::from_raw_parts(value.pixels, value.width as usize * value.height as usize)
        };
        Sprite { info, pixels }
    }
}
impl Drop for Fpg {
    fn drop(&mut self) {
        // SAFETY: unique native owner; borrowed views cannot outlive it.
        unsafe {
            ffi::mv_fpg_destroy(self.native.as_ptr());
        }
    }
}
fn point_value(value: ffi::MvFpgPoint) -> Point {
    Point {
        x: value.x,
        y: value.y,
        defined: value.defined != 0,
    }
}
fn parse_error(error: *const c_char) -> Error {
    if error.is_null() {
        return Error::Asset("FPG parse failed");
    }
    // SAFETY: called only for native parser failure with a static diagnostic.
    let error: &'static CStr = unsafe { CStr::from_ptr(error) };
    Error::Asset(error.to_str().unwrap_or("FPG parse failed"))
}
/// Borrowed indexes cannot outlive their native data.
/// ```compile_fail
/// use malvado::fpg::Fpg;
/// fn invalid(data: &[u8]) -> &[u8] {
///     let fpg = Fpg::parse(data).unwrap();
///     fpg.sprite(7).unwrap().pixels
/// }
/// ```
/// Native ownership and cleanup stay on the creating thread.
/// ```compile_fail
/// use malvado::fpg::Fpg;
/// fn invalid(fpg: Fpg) { std::thread::spawn(move || drop(fpg)); }
/// ```
mod ownership_contract {}

impl Fpg {
    /// Return RGBA with six-bit palette channels shifted left by two. No gamma correction.
    pub fn rgba(&self, id: i32, transparent_zero: bool) -> Result<Vec<u8>, Error> {
        let sprite = self.sprite(id).ok_or(Error::NotFound)?;
        let length = sprite.pixels.len().checked_mul(4).ok_or(Error::Overflow)?;
        copied(length, |out| {
            // SAFETY: separate initialized output with exactly the required size.
            unsafe {
                ffi::mv_fpg_copy_rgba(
                    self.native.as_ptr(),
                    id,
                    transparent_zero.into(),
                    out.as_mut_ptr(),
                    out.len(),
                )
            }
        })
    }
    /// Encode an indexed PNG without creating a file. Palette indexes are preserved.
    pub fn png(&self, id: i32, transparent_zero: bool) -> Result<Vec<u8>, Error> {
        self.sprite(id).ok_or(Error::NotFound)?;
        // SAFETY: live immutable native library and a typed boolean policy.
        let length =
            unsafe { ffi::mv_fpg_png_size(self.native.as_ptr(), id, transparent_zero.into()) };
        copied(length, |out| {
            // SAFETY: native sizing precedes the copy, with no source aliasing.
            unsafe {
                ffi::mv_fpg_copy_png(
                    self.native.as_ptr(),
                    id,
                    transparent_zero.into(),
                    out.as_mut_ptr(),
                    out.len(),
                )
            }
        })
    }
    /// Encode lossless native metadata JSON. The returned string excludes the C terminator.
    pub fn json(&self, transparent_zero: bool) -> Result<String, Error> {
        // SAFETY: live immutable native library; size includes the terminator.
        let length =
            unsafe { ffi::mv_fpg_json_size(self.native.as_ptr(), transparent_zero.into()) };
        let mut bytes = copied(length, |out| {
            // SAFETY: distinct correctly sized output buffer; C writes valid UTF-8 JSON.
            unsafe {
                ffi::mv_fpg_copy_json(
                    self.native.as_ptr(),
                    transparent_zero.into(),
                    out.as_mut_ptr().cast(),
                    out.len(),
                )
            }
        })?;
        if bytes.pop() != Some(0) {
            return Err(Error::InvalidArgument);
        }
        String::from_utf8(bytes).map_err(|_| Error::InvalidArgument)
    }
    /// Create an independently owned image; the FPG data need not remain alive.
    #[cfg(feature = "graphics")]
    pub fn image(
        &self,
        window: Rc<crate::window::Window>,
        id: i32,
        transparent_zero: bool,
    ) -> Result<crate::image::Image, Error> {
        let info = self.sprite(id).ok_or(Error::NotFound)?.info;
        let rgba = self.rgba(id, transparent_zero)?;
        crate::image::Image::from_rgba(window, info.width, info.height, &rgba)
    }
}
fn copied(length: usize, copy: impl FnOnce(&mut [u8]) -> i32) -> Result<Vec<u8>, Error> {
    if length == 0 {
        return Err(Error::InvalidArgument);
    }
    let mut out = Vec::new();
    out.try_reserve_exact(length)
        .map_err(|_| Error::OutOfMemory)?;
    out.resize(length, 0);
    if copy(&mut out) != 1 {
        return Err(Error::InvalidArgument);
    }
    Ok(out)
}

/// Owned immutable converted-FPG metadata. No pixels or PNG files are loaded.
pub struct Manifest {
    native: NonNull<ffi::MvFpgManifest>,
    local: PhantomData<Rc<()>>,
}
impl Manifest {
    /// Take one uniquely owned, fully validated native allocation.
    /// SAFETY: the pointer must come from the matching native importer and
    /// must have no other owning wrapper or pending destruction.
    pub(crate) unsafe fn from_native(native: NonNull<ffi::MvFpgManifest>) -> Self {
        Self {
            native,
            local: PhantomData,
        }
    }

    pub fn parse(json: &str) -> Result<Self, Error> {
        sys::check_abi().map_err(Error::Abi)?;
        let mut error = std::ptr::null();
        let _json = crate::asset::json_guard();
        // SAFETY: the native parser borrows JSON for this call, owns its result,
        // and returns static diagnostics only.
        let native =
            unsafe { ffi::mv_fpg_manifest_parse(json.as_ptr().cast(), json.len(), &mut error) };
        Ok(Self {
            native: NonNull::new(native).ok_or_else(|| parse_error(error))?,
            local: PhantomData,
        })
    }
    pub fn len(&self) -> usize {
        // SAFETY: live owned manifest.
        unsafe { ffi::mv_fpg_manifest_count(self.native.as_ptr()) }
    }
    pub fn is_empty(&self) -> bool {
        self.len() == 0
    }
    pub fn transparent_zero(&self) -> bool {
        // SAFETY: live manifests return only 0 or 1, never the NULL sentinel.
        unsafe { ffi::mv_fpg_manifest_transparent_zero(self.native.as_ptr()) == 1 }
    }
    pub fn palette(&self) -> &[u8; 768] {
        // SAFETY: a live manifest owns the entire fixed-size block.
        unsafe { &*ffi::mv_fpg_manifest_palette(self.native.as_ptr()).cast::<[u8; 768]>() }
    }
    pub fn gamma(&self) -> &[u8; 576] {
        // SAFETY: a live manifest owns the entire fixed-size block.
        unsafe { &*ffi::mv_fpg_manifest_gamma(self.native.as_ptr()).cast::<[u8; 576]>() }
    }
    pub fn sprite(&self, id: i32) -> Option<SpriteInfo> {
        let mut out = MaybeUninit::uninit();
        // SAFETY: output is initialized only on lookup success.
        if unsafe { ffi::mv_fpg_manifest_sprite(self.native.as_ptr(), id, out.as_mut_ptr()) } == 0 {
            return None;
        }
        // SAFETY: successful lookup initialized the whole record.
        Some(manifest_value(unsafe { out.assume_init() }))
    }
    pub fn at(&self, index: usize) -> Option<SpriteInfo> {
        let mut out = MaybeUninit::uninit();
        // SAFETY: native enumeration validates the index before writing output.
        if unsafe { ffi::mv_fpg_manifest_at(self.native.as_ptr(), index, out.as_mut_ptr()) } == 0 {
            return None;
        }
        // SAFETY: successful lookup initialized the whole record.
        Some(manifest_value(unsafe { out.assume_init() }))
    }
    pub fn point(&self, id: i32, index: usize) -> Option<Point> {
        let mut out = MaybeUninit::uninit();
        // SAFETY: valid output storage, initialized only on native success.
        if unsafe { ffi::mv_fpg_manifest_point(self.native.as_ptr(), id, index, out.as_mut_ptr()) }
            == 0
        {
            return None;
        }
        // SAFETY: successful lookup initialized the whole record.
        Some(point_value(unsafe { out.assume_init() }))
    }
}
impl Drop for Manifest {
    fn drop(&mut self) {
        // SAFETY: release the unique native allocation once after borrows end.
        unsafe {
            ffi::mv_fpg_manifest_destroy(self.native.as_ptr());
        }
    }
}
fn manifest_value(value: ffi::MvFpgManifestSprite) -> SpriteInfo {
    SpriteInfo {
        id: value.id,
        width: value.width,
        height: value.height,
        name: value.name,
        filename: value.filename,
        point_count: value.point_count,
    }
}
/// Manifest palette borrows cannot escape their owner.
/// ```compile_fail
/// use malvado::fpg::Manifest;
/// fn invalid(json: &str) -> &[u8;768] {
///     let manifest = Manifest::parse(json).unwrap();
///     manifest.palette()
/// }
/// ```
/// Native metadata destruction stays on the creating thread.
/// ```compile_fail
/// use malvado::fpg::Manifest;
/// fn invalid(manifest: Manifest) { std::thread::spawn(move || drop(manifest)); }
/// ```
mod manifest_ownership_contract {}
