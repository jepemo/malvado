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

/// Complete native library metadata. Each variant owns its native allocation.
/// Atlas also represents imported Aseprite metadata. No variant owns textures.
pub enum Metadata {
    Atlas(crate::atlas::Atlas),
    Fpg(crate::fpg::Fpg),
    Manifest(crate::fpg::Manifest),
}

/// An owned source and its validated library metadata. Files may be removed
/// after opening. PNG files use Source and an image loader instead.
/// ```
/// use malvado::asset::{Library, Metadata};
/// let path = std::env::temp_dir().join(format!("malvado-library-doc-{}.json", std::process::id()));
/// std::fs::write(&path, r#"{"schema":1,"kind":"graphics-library","image":"sheet.png","sprites":[{"id":7,"name":"ship","x":0,"y":0,"width":4,"height":8}]}"#)?;
/// let library = Library::open(&path, None)?;
/// std::fs::remove_file(path)?;
/// let Metadata::Atlas(atlas) = library.into_metadata() else { unreachable!() };
/// assert_eq!(atlas.sprite(7).unwrap().name, "ship");
/// # Ok::<(), Box<dyn std::error::Error>>(())
/// ```
/// ```compile_fail
/// let library = malvado::asset::Library::open("sheet.json", None).unwrap();
/// std::thread::spawn(move || drop(library));
/// ```
pub struct Library {
    source: Source,
    metadata: Metadata,
    format: Format,
}
struct ParsedMetadata(ffi::MvAssetMetadata);
impl Drop for ParsedMetadata {
    fn drop(&mut self) {
        // SAFETY: this guard owns all pointers that were not moved out.
        unsafe {
            ffi::mv_asset_metadata_destroy(&mut self.0);
        }
    }
}
impl Format {
    fn native(self) -> i32 {
        match self {
            Self::Unknown => ffi::MV_ASSET_UNKNOWN,
            Self::Png => ffi::MV_ASSET_PNG,
            Self::Fpg => ffi::MV_ASSET_FPG,
            Self::Atlas => ffi::MV_ASSET_ATLAS,
            Self::Aseprite => ffi::MV_ASSET_ASEPRITE,
            Self::FpgManifest => ffi::MV_ASSET_FPG_MANIFEST,
        }
    }
}
impl Library {
    /// Open and fully parse metadata with the native dispatcher. None or
    /// Unknown uses the source hint; other formats override it. Actual texture
    /// bounds are not validated without an image. Source bytes stay owned.
    pub fn open(path: impl AsRef<Path>, format: Option<Format>) -> Result<Self, Error> {
        Self::from_source(Source::open(path)?, format)
    }
    /// Consume an existing snapshot. Failure drops the source and any partial
    /// metadata. JSON parsing uses the same lock as other safe import calls.
    pub fn from_source(source: Source, format: Option<Format>) -> Result<Self, Error> {
        let mut parsed = ParsedMetadata(ffi::MvAssetMetadata::default());
        let mut error = std::ptr::null();
        let _json = json_guard();
        // SAFETY: borrowed source, distinct empty output and error pointer.
        // Native failure leaves the guard empty and returns a static message.
        if unsafe {
            ffi::mv_asset_metadata_parse(
                source.native.as_ptr(),
                format.unwrap_or(Format::Unknown).native(),
                &mut parsed.0,
                &mut error,
            )
        } != 1
        {
            let message = if error.is_null() {
                "could not parse asset metadata"
            } else {
                // SAFETY: the native contract provides immutable static errors.
                let message: &'static CStr = unsafe { CStr::from_ptr(error) };
                message.to_str().unwrap_or("could not parse asset metadata")
            };
            return Err(Error::Asset(message));
        }
        let format = match parsed.0.format {
            ffi::MV_ASSET_ATLAS => Format::Atlas,
            ffi::MV_ASSET_ASEPRITE => Format::Aseprite,
            ffi::MV_ASSET_FPG => Format::Fpg,
            ffi::MV_ASSET_FPG_MANIFEST => Format::FpgManifest,
            other => return Err(Error::UnexpectedNative(other)),
        };
        // SAFETY: successful native dispatch returns exactly one validated
        // owned pointer of the selected type. Clear it before guard cleanup.
        let metadata = unsafe {
            match format {
                Format::Atlas | Format::Aseprite => {
                    let native =
                        NonNull::new(std::mem::replace(&mut parsed.0.atlas, std::ptr::null_mut()))
                            .expect("native atlas metadata");
                    Metadata::Atlas(crate::atlas::Atlas::from_native(native))
                }
                Format::Fpg => {
                    let native =
                        NonNull::new(std::mem::replace(&mut parsed.0.fpg, std::ptr::null_mut()))
                            .expect("native FPG metadata");
                    Metadata::Fpg(crate::fpg::Fpg::from_native(native))
                }
                Format::FpgManifest => {
                    let native = NonNull::new(std::mem::replace(
                        &mut parsed.0.manifest,
                        std::ptr::null_mut(),
                    ))
                    .expect("native manifest metadata");
                    Metadata::Manifest(crate::fpg::Manifest::from_native(native))
                }
                _ => unreachable!(),
            }
        };
        Ok(Self {
            source,
            metadata,
            format,
        })
    }
    pub fn source(&self) -> &Source {
        &self.source
    }
    pub fn metadata(&self) -> &Metadata {
        &self.metadata
    }
    pub fn format(&self) -> Format {
        self.format
    }
    /// Release the file snapshot and return independently owned metadata.
    pub fn into_metadata(self) -> Metadata {
        self.metadata
    }
}
