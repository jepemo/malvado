// Malvado 2; SPDX-License-Identifier: Apache-2.0
use std::sync::{Mutex, MutexGuard};

// cJSON writes one global error record even when callers do not read it.
// Serialize safe Rust JSON import calls; no user callbacks run under this lock.
static JSON_PARSE: Mutex<()> = Mutex::new(());

pub(crate) fn json_guard() -> MutexGuard<'static, ()> {
    // No Rust data is shared behind the guard. Poison does not invalidate the
    // native parser, whose error record is reset on its next call.
    JSON_PARSE
        .lock()
        .unwrap_or_else(|poison| poison.into_inner())
}

use crate::Error;
use malvado_sys::{self as sys, assets as ffi};
use std::{
    ffi::{CStr, CString, OsStr, OsString},
    marker::PhantomData,
    os::unix::ffi::{OsStrExt, OsStringExt},
    path::{Path, PathBuf},
    ptr::NonNull,
    rc::Rc,
};

/// Content routing hint. Importers must still validate the complete format.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Format {
    Unknown,
    Png,
    Fpg,
    Atlas,
    Aseprite,
    FpgManifest,
}

/// Owned native file snapshot, limited to 64 MiB. No window or context needed.
/// The extra native NUL terminator is excluded from bytes(). Creating and
/// destroying the source stay on its owner thread.
/// ```
/// use malvado::asset::{Source, Format};
/// let path = std::env::temp_dir().join(format!("malvado-source-doc-{}.json", std::process::id()));
/// std::fs::write(&path, r#"{"kind":"graphics-library"}"#)?;
/// let source = Source::open(&path)?;
/// std::fs::remove_file(&path)?;
/// assert_eq!(source.format(), Format::Atlas);
/// assert_eq!(source.bytes(), br#"{"kind":"graphics-library"}"#);
/// # Ok::<(), Box<dyn std::error::Error>>(())
/// ```
/// ```compile_fail
/// let source = malvado::asset::Source::open("sheet.json").unwrap();
/// std::thread::spawn(move || drop(source));
/// ```
pub struct Source {
    native: NonNull<ffi::MvAssetSource>,
    local: PhantomData<Rc<()>>,
}
impl Source {
    pub fn open(path: impl AsRef<Path>) -> Result<Self, Error> {
        sys::check_abi().map_err(Error::Abi)?;
        let path = CString::new(path.as_ref().as_os_str().as_bytes())
            .map_err(|_| Error::InvalidArgument)?;
        let mut error = std::ptr::null();
        let _json = json_guard();
        // SAFETY: borrowed path/output, no callbacks. Native owns a copied path
        // and file snapshot; this guard serializes cJSON's global error state.
        let native = unsafe { ffi::mv_asset_source_open(path.as_ptr(), &mut error) };
        let native = NonNull::new(native).ok_or_else(|| {
            let message = if error.is_null() {
                "asset source open failed"
            } else {
                // SAFETY: native errors are immutable static strings.
                let message: &'static CStr = unsafe { CStr::from_ptr(error) };
                message.to_str().unwrap_or("asset source open failed")
            };
            Error::Asset(message)
        })?;
        Ok(Self {
            native,
            local: PhantomData,
        })
    }
    /// Borrow the complete copied source, including any embedded NUL bytes.
    pub fn bytes(&self) -> &[u8] {
        let mut length = 0;
        // SAFETY: a live source owns non-null storage, bounded to 64 MiB.
        let bytes = unsafe { ffi::mv_asset_source_data(self.native.as_ptr(), &mut length) };
        // SAFETY: the native allocation remains borrowed for self's lifetime.
        unsafe { std::slice::from_raw_parts(bytes.cast(), length) }
    }
    /// Original path, preserved without canonicalization or UTF-8 conversion.
    pub fn path(&self) -> &Path {
        // SAFETY: live native source owns a NUL-terminated copied path.
        let bytes =
            unsafe { CStr::from_ptr(ffi::mv_asset_source_path(self.native.as_ptr())) }.to_bytes();
        Path::new(OsStr::from_bytes(bytes))
    }
    pub fn format(&self) -> Format {
        // SAFETY: immutable copied source metadata.
        match unsafe { ffi::mv_asset_source_format(self.native.as_ptr()) } {
            ffi::MV_ASSET_PNG => Format::Png,
            ffi::MV_ASSET_FPG => Format::Fpg,
            ffi::MV_ASSET_ATLAS => Format::Atlas,
            ffi::MV_ASSET_ASEPRITE => Format::Aseprite,
            ffi::MV_ASSET_FPG_MANIFEST => Format::FpgManifest,
            _ => Format::Unknown,
        }
    }
    /// Advisory Aseprite meta.size, not validated image dimensions.
    pub fn dimensions(&self) -> Option<(i32, i32)> {
        let (mut width, mut height) = (0, 0);
        // SAFETY: distinct output values and immutable live source.
        (unsafe { ffi::mv_asset_source_dimensions(self.native.as_ptr(), &mut width, &mut height) }
            == 1)
            .then_some((width, height))
    }
    /// Join an image reference to the source directory. Does not open a file.
    pub fn image_path(&self, image: impl AsRef<Path>) -> Result<PathBuf, Error> {
        let image = CString::new(image.as_ref().as_os_str().as_bytes())
            .map_err(|_| Error::InvalidArgument)?;
        // SAFETY: live source and borrowed, NUL-terminated image path.
        let size =
            unsafe { ffi::mv_asset_source_image_path_size(self.native.as_ptr(), image.as_ptr()) };
        if size == 0 {
            return Err(Error::InvalidArgument);
        }
        let mut bytes = Vec::new();
        bytes
            .try_reserve_exact(size)
            .map_err(|_| Error::OutOfMemory)?;
        bytes.resize(size, 0);
        // SAFETY: exact-size distinct output, no native retention.
        if unsafe {
            ffi::mv_asset_source_image_path(
                self.native.as_ptr(),
                image.as_ptr(),
                bytes.as_mut_ptr().cast(),
                size,
            )
        } != 1
        {
            return Err(Error::InvalidArgument);
        }
        bytes.pop();
        Ok(PathBuf::from(OsString::from_vec(bytes)))
    }
}
impl Drop for Source {
    fn drop(&mut self) {
        // SAFETY: unique native owner; borrowed bytes/paths cannot outlive self.
        unsafe {
            ffi::mv_asset_source_destroy(self.native.as_ptr());
        }
    }
}
