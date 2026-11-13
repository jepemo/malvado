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

/// An atlas paired with a validated native image. Both stay on the window
/// thread. Construction consumes metadata and shares the immutable texture.
#[cfg(feature = "graphics")]
pub struct TextureAtlas {
    atlas: Atlas,
    image: crate::image::Image,
}
#[cfg(feature = "graphics")]
impl TextureAtlas {
    /// Reject image bounds before retaining either value. Aseprite dimensions
    /// must match exactly. Failure drops the supplied owners normally.
    pub fn new(atlas: Atlas, image: crate::image::Image) -> Result<Self, Error> {
        let (width, height) = image.size();
        if !atlas.validate_image(width, height) {
            return Err(Error::InvalidArgument);
        }
        Ok(Self { atlas, image })
    }
    /// Load the referenced sheet relative to the metadata source path. Accept
    /// only atlas/Aseprite libraries; other formats return InvalidArgument.
    /// Image loading follows the native path cache; metadata uses the snapshot.
    pub fn load(
        window: Rc<crate::window::Window>,
        library: crate::asset::Library,
    ) -> Result<Self, Error> {
        let crate::asset::Metadata::Atlas(atlas) = library.metadata() else {
            return Err(Error::InvalidArgument);
        };
        let path = library.source().image_path(atlas.image())?;
        let image = crate::image::Image::load(window, path)?;
        let crate::asset::Metadata::Atlas(atlas) = library.into_metadata() else {
            unreachable!()
        };
        Self::new(atlas, image)
    }
    pub fn metadata(&self) -> &Atlas {
        &self.atlas
    }
    pub fn image(&self) -> &crate::image::Image {
        &self.image
    }
}
/// Textured atlases retain owner-thread window resources.
/// ```compile_fail
/// fn invalid(atlas: malvado::atlas::TextureAtlas) {
///     std::thread::spawn(move || drop(atlas));
/// }
/// ```
#[cfg(feature = "graphics")]
mod texture_ownership_contract {}

/// An immutable selected sprite. Cloning shares the image; atlas metadata may
/// be dropped immediately. The pivot is the native cropped-image pivot.
#[cfg(feature = "graphics")]
#[derive(Clone)]
pub struct SpriteImage {
    image: crate::image::Image,
    region: Region,
    pivot: (f64, f64),
}
#[cfg(feature = "graphics")]
impl TextureAtlas {
    pub fn sprite(&self, id: i32) -> Option<SpriteImage> {
        self.atlas.sprite(id).map(|sprite| self.select(sprite))
    }
    /// Exact case-sensitive name; embedded NUL returns InvalidArgument.
    pub fn sprite_named(&self, name: &str) -> Result<Option<SpriteImage>, Error> {
        Ok(self
            .atlas
            .sprite_named(name)?
            .map(|sprite| self.select(sprite)))
    }
    fn select(&self, sprite: Sprite<'_>) -> SpriteImage {
        SpriteImage {
            image: self.image.clone(),
            region: sprite.region,
            pivot: (f64::from(sprite.pivot.0), f64::from(sprite.pivot.1)),
        }
    }
}
#[cfg(feature = "graphics")]
impl SpriteImage {
    pub(crate) fn whole(image: crate::image::Image, pivot: (f64, f64)) -> Self {
        let (width, height) = image.size();
        Self {
            image,
            region: Region {
                x: 0,
                y: 0,
                width,
                height,
            },
            pivot,
        }
    }
    fn native_graphic(&self) -> malvado_sys::graphics::MvImageGraphic {
        malvado_sys::graphics::MvImageGraphic {
            image: self.image.native(),
            region: malvado_sys::graphics::MvImageRegion {
                x: self.region.x,
                y: self.region.y,
                width: self.region.width,
                height: self.region.height,
            },
            pivot_x: self.pivot.0,
            pivot_y: self.pivot.1,
        }
    }
    pub fn region(&self) -> Region {
        self.region
    }
    pub fn pivot(&self) -> (f64, f64) {
        self.pivot
    }
    /// Local collider only; does not register a process or copy pixel data.
    pub fn shape(&self) -> Result<crate::collision::Shape, Error> {
        crate::collision::Shape::sprite((self.region.width, self.region.height), self.pivot)
    }
}
#[cfg(feature = "graphics")]
impl crate::window::Frame<'_> {
    /// Draw a selected sprite with its native pivot. Scale must be positive;
    /// position, scale and clockwise rotation must be finite and renderable.
    /// Frame retention keeps the texture alive after the sprite is dropped.
    pub fn sprite(
        &self,
        sprite: &SpriteImage,
        position: (f32, f32),
        scale: f32,
        rotation: f32,
    ) -> Result<(), Error> {
        self.image_region(
            &sprite.image,
            sprite.region.into(),
            crate::image::Transform {
                position,
                pivot: (sprite.pivot.0 as f32, sprite.pivot.1 as f32),
                scale,
                rotation,
            },
        )
    }
}

/// Copied selected sprite geometry. This value owns no image or window.
#[cfg(feature = "graphics")]
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct SpriteGeometry {
    pub region: Region,
    pub pivot: (f64, f64),
}
#[cfg(feature = "graphics")]
impl crate::Handle {
    /// Select a sprite through native graph storage. The transferred image owner
    /// retains its window until replacement, clear or process/context cleanup.
    pub fn set_sprite(&self, sprite: &SpriteImage) -> Result<(), Error> {
        let ctx = self.inner.runtime.live()?;
        let owner = Box::into_raw(Box::new(sprite.image.clone()));
        let graphic = sprite.native_graphic();
        // SAFETY: live context, validated immutable sprite, owned boxed image.
        // Native success takes owner; failure retains neither owner nor image.
        let status = unsafe {
            malvado_sys::graphics::mv_process_image_set_owned(
                ctx,
                self.inner.reference,
                graphic,
                owner.cast(),
                Some(release_sprite_owner),
            )
        };
        if let Err(error) = crate::engine::result(status) {
            // SAFETY: failed native calls leave the boxed owner with us.
            unsafe {
                drop(Box::from_raw(owner));
            }
            return Err(error);
        }
        Ok(())
    }
    /// Read copied native geometry. The borrowed native image is not exposed.
    /// Primitive or absent graphs return None; clear_graphic clears either kind.
    pub fn sprite_geometry(&self) -> Result<Option<SpriteGeometry>, Error> {
        let mut out = std::mem::MaybeUninit::uninit();
        let mut present = 0;
        // SAFETY: separate outputs; native only initializes out when present.
        crate::engine::result(unsafe {
            malvado_sys::graphics::mv_process_image_get(
                self.inner.runtime.live()?,
                self.inner.reference,
                out.as_mut_ptr(),
                &mut present,
            )
        })?;
        if present == 0 {
            return Ok(None);
        }
        // SAFETY: successful present result initialized the complete descriptor.
        let out = unsafe { out.assume_init() };
        Ok(Some(SpriteGeometry {
            region: Region {
                x: out.region.x,
                y: out.region.y,
                width: out.region.width,
                height: out.region.height,
            },
            pivot: (out.pivot_x, out.pivot_y),
        }))
    }
}
#[cfg(feature = "graphics")]
unsafe extern "C" fn release_sprite_owner(data: *mut std::ffi::c_void) {
    // SAFETY: native success transferred this Box<Image>, released exactly once
    // after native image uses. Containment prevents unwinding across C.
    let _ = crate::task::contain(|| unsafe {
        drop(Box::from_raw(data.cast::<crate::image::Image>()));
    });
}

/// One owned sprite and its duration in microseconds, without a separate clock.
#[cfg(feature = "graphics")]
#[derive(Clone)]
pub struct SpriteFrame {
    pub sprite: SpriteImage,
    pub duration_us: u64,
}
#[cfg(feature = "graphics")]
impl crate::Handle {
    /// Copy and retain 1..8192 visual frames in the native process. Durations
    /// are 1..3600000000 us; playback uses Game's 60 Hz ticks. The transferred
    /// image owners keep the window alive after the source frames are dropped.
    pub fn set_sprite_animation(&self, frames: &[SpriteFrame], looping: bool) -> Result<(), Error> {
        let ctx = self.inner.runtime.live()?;
        if frames.is_empty()
            || frames.len() > 8192
            || frames
                .iter()
                .any(|frame| frame.duration_us == 0 || frame.duration_us > 3_600_000_000)
        {
            return Err(Error::InvalidArgument);
        }
        let mut native = Vec::new();
        let mut images = Vec::new();
        native
            .try_reserve_exact(frames.len())
            .map_err(|_| Error::OutOfMemory)?;
        images
            .try_reserve_exact(frames.len())
            .map_err(|_| Error::OutOfMemory)?;
        for frame in frames {
            native.push(malvado_sys::graphics::MvImageAnimationFrame {
                graphic: frame.sprite.native_graphic(),
                duration_us: frame.duration_us,
            });
            images.push(frame.sprite.image.clone());
        }
        let owner = Box::into_raw(Box::new(images));
        // SAFETY: live context and immutable borrowed frames. Success copies
        // descriptors, retains images and takes the boxed owner; failure takes
        // nothing. The callback runs only after all native image uses end.
        let status = unsafe {
            malvado_sys::graphics::mv_process_image_animation_set_owned(
                ctx,
                self.inner.reference,
                native.as_ptr(),
                native.len(),
                looping.into(),
                60,
                owner.cast(),
                Some(release_animation_owner),
            )
        };
        if let Err(error) = crate::engine::result(status) {
            // SAFETY: native failure leaves this unique owner with the caller.
            unsafe {
                drop(Box::from_raw(owner));
            }
            return Err(error);
        }
        Ok(())
    }
}
#[cfg(feature = "graphics")]
unsafe extern "C" fn release_animation_owner(data: *mut std::ffi::c_void) {
    // SAFETY: native success owns this Box<Vec<Image>> and releases it once.
    let _ = crate::task::contain(|| unsafe {
        drop(Box::from_raw(data.cast::<Vec<crate::image::Image>>()));
    });
}
