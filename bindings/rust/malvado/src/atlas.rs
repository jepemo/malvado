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
    /// Take one uniquely owned, fully validated native allocation.
    /// SAFETY: the pointer must come from the matching native importer and
    /// must have no other owning wrapper or pending destruction.
    pub(crate) unsafe fn from_native(native: NonNull<ffi::MvAtlas>) -> Self {
        Self {
            native,
            local: PhantomData,
        }
    }

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
        let _json = crate::asset::json_guard();
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
    /// Validate actual image dimensions without reparsing or mutating metadata.
    /// Aseprite requires the declared sheet size; other atlases require regions
    /// to fit. Invalid dimensions return false. No window is needed.
    pub fn validate_image(&self, width: i32, height: i32) -> bool {
        // SAFETY: live immutable atlas and copied dimensions, no retention.
        unsafe { ffi::mv_atlas_validate_image(self.native.as_ptr(), width, height) == 1 }
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
/// Animation views also borrow the owning atlas.
/// ```compile_fail
/// use malvado::atlas::Atlas;
/// fn invalid(json: &str) {
///     let clip = {
///         let atlas = Atlas::parse(json, 8, 8).unwrap();
///         atlas.animation_named("walk").unwrap().unwrap()
///     };
///     clip.sample(0, true);
/// }
/// ```
/// Atlas access stays on its creating thread.
/// ```compile_fail
/// use malvado::atlas::Atlas;
/// fn invalid(atlas: Atlas) { std::thread::spawn(move || drop(atlas)); }
/// ```
mod ownership_contract {}

/// One expanded sequence frame. Durations use the atlas format's milliseconds.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct ClipFrame {
    pub sprite_id: i32,
    pub duration_ms: u32,
}
/// A named expanded sequence borrowing its owning atlas.
pub struct Clip<'a> {
    atlas: &'a Atlas,
    name: CString,
    count: usize,
}
impl Atlas {
    /// Import an unrotated Aseprite array export with native validation.
    pub fn parse_aseprite(json: &str, width: i32, height: i32) -> Result<Self, Error> {
        Self::parse_with(json, width, height, ffi::mv_atlas_parse_aseprite)
    }
    /// Optional imported pivot-slice color as RGBA channels.
    pub fn pivot_color(&self) -> Option<[u8; 4]> {
        let mut out = MaybeUninit::uninit();
        // SAFETY: live atlas, exclusive output initialized only on success.
        if unsafe { ffi::mv_atlas_pivot_color(self.native.as_ptr(), out.as_mut_ptr()) } == 0 {
            return None;
        }
        // SAFETY: successful query initializes all four channels.
        Some(color(unsafe { out.assume_init() }))
    }
    pub fn animation_named(&self, name: &str) -> Result<Option<Clip<'_>>, Error> {
        let name = CString::new(name).map_err(|_| Error::InvalidArgument)?;
        // SAFETY: live atlas and terminated name, no retained caller pointers.
        let count =
            unsafe { ffi::mv_atlas_animation_frame_count(self.native.as_ptr(), name.as_ptr()) };
        Ok((count != 0).then_some(Clip {
            atlas: self,
            name,
            count,
        }))
    }
}
impl Clip<'_> {
    pub fn len(&self) -> usize {
        self.count
    }
    pub fn is_empty(&self) -> bool {
        self.count == 0
    }
    pub fn frame(&self, index: usize) -> Option<ClipFrame> {
        let mut out = MaybeUninit::uninit();
        // SAFETY: atlas borrow and owned C name stay live; native validates index.
        if unsafe {
            ffi::mv_atlas_animation_frame_at(
                self.atlas.native.as_ptr(),
                self.name.as_ptr(),
                index,
                out.as_mut_ptr(),
            )
        } == 0
        {
            return None;
        }
        // SAFETY: successful query initialized both output fields.
        let out = unsafe { out.assume_init() };
        Some(ClipFrame {
            sprite_id: out.sprite_id,
            duration_ms: out.duration_ms,
        })
    }
    /// Sample the expanded native sequence; non-looping clips hold the last frame.
    pub fn sample(&self, elapsed_ms: u64, looping: bool) -> i32 {
        let mut id = 0;
        // SAFETY: immutable atlas keeps this known nonempty sequence live.
        let valid = unsafe {
            ffi::mv_atlas_animation_frame(
                self.atlas.native.as_ptr(),
                self.name.as_ptr(),
                elapsed_ms,
                looping.into(),
                &mut id,
            )
        };
        assert_eq!(valid, 1, "live atlas sequence");
        id
    }
    pub fn color(&self) -> Option<[u8; 4]> {
        let mut out = MaybeUninit::uninit();
        // SAFETY: live atlas and name; output initialized only on success.
        if unsafe {
            ffi::mv_atlas_animation_color(
                self.atlas.native.as_ptr(),
                self.name.as_ptr(),
                out.as_mut_ptr(),
            )
        } == 0
        {
            return None;
        }
        // SAFETY: successful query initializes all four channels.
        Some(color(unsafe { out.assume_init() }))
    }
    /// Explicit repeat metadata. None means unspecified; zero is preserved.
    /// Native sampling and converted playback do not apply this count.
    pub fn repeats(&self) -> Option<u32> {
        let mut out = 0;
        // SAFETY: live atlas, owned name and exclusive output.
        let valid = unsafe {
            ffi::mv_atlas_animation_repeats(
                self.atlas.native.as_ptr(),
                self.name.as_ptr(),
                &mut out,
            )
        };
        (valid != 0).then_some(out)
    }
    /// Copy expanded frames into an independent owned native animation.
    /// Graph IDs remain metadata and do not retain images. Repeat count is not applied.
    pub fn to_animation(&self, looping: bool) -> Result<crate::animation::Animation, Error> {
        let frames: Vec<_> = (0..self.count)
            .map(|index| {
                let frame = self.frame(index).expect("live expanded atlas sequence");
                crate::animation::Frame {
                    graph_id: frame.sprite_id,
                    duration_us: u64::from(frame.duration_ms) * 1000,
                }
            })
            .collect();
        crate::animation::Animation::new(&frames, looping)
    }
}
fn color(value: ffi::MvColor) -> [u8; 4] {
    [value.r, value.g, value.b, value.a]
}
