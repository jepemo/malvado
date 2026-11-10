// Malvado 2; SPDX-License-Identifier: Apache-2.0
//! Owned native images retain their window and any queued frame uses.
use crate::{
    Error,
    drawing::{Rectangle, valid},
    window::{Frame, Window},
};
use malvado_sys::graphics as ffi;
use std::{ffi::CString, os::unix::ffi::OsStrExt, path::Path, ptr::NonNull, rc::Rc};

/// Position and pivot use pixels; rotation is clockwise and scale is positive.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Transform {
    pub position: (f32, f32),
    pub pivot: (f32, f32),
    pub scale: f32,
    pub rotation: f32,
}
impl Default for Transform {
    fn default() -> Self {
        Self {
            position: (0.0, 0.0),
            pivot: (0.0, 0.0),
            scale: 1.0,
            rotation: 0.0,
        }
    }
}
impl Transform {
    fn native(self) -> ffi::MvImageTransform {
        ffi::MvImageTransform {
            x: self.position.0,
            y: self.position.1,
            pivot_x: self.pivot.0,
            pivot_y: self.pivot.1,
            scale: self.scale,
            rotation: self.rotation,
        }
    }
}
/// Clone shares this immutable owned reference. Native loads also use the C
/// path cache. Neither operation copies pixels or creates another texture.
#[derive(Clone)]
pub struct Image {
    inner: Rc<Inner>,
}
struct Inner {
    native: NonNull<ffi::MvImage>,
    window: Rc<Window>,
}
impl Image {
    pub fn load(window: Rc<Window>, path: impl AsRef<Path>) -> Result<Self, Error> {
        let path = CString::new(path.as_ref().as_os_str().as_bytes())
            .map_err(|_| Error::InvalidArgument)?;
        // SAFETY: retained owner-thread window; native load copies the path and
        // returns one owned reference, including for an existing cached image.
        let native = unsafe { ffi::mv_image_load(path.as_ptr()) };
        Self::owned(native, window)
    }
    /// Copy packed RGBA bytes into a native texture. No caller buffer is retained.
    pub fn from_rgba(
        window: Rc<Window>,
        width: i32,
        height: i32,
        pixels: &[u8],
    ) -> Result<Self, Error> {
        if !(1..=8192).contains(&width)
            || !(1..=8192).contains(&height)
            || pixels.len() < width as usize * height as usize * 4
        {
            return Err(Error::InvalidArgument);
        }
        // SAFETY: positive bounded dimensions, sufficient bytes and live window.
        Self::owned(
            unsafe { ffi::mv_image_from_rgba(width, height, pixels.as_ptr(), pixels.len()) },
            window,
        )
    }
    fn owned(native: *mut ffi::MvImage, window: Rc<Window>) -> Result<Self, Error> {
        Ok(Self {
            inner: Rc::new(Inner {
                native: NonNull::new(native)
                    .ok_or(Error::Platform("image load or creation failed"))?,
                window,
            }),
        })
    }
    pub fn size(&self) -> (i32, i32) {
        // SAFETY: the shared owner retains both native image and its window.
        unsafe {
            (
                ffi::mv_image_width(self.inner.native.as_ptr()),
                ffi::mv_image_height(self.inner.native.as_ptr()),
            )
        }
    }
}
impl Drop for Inner {
    fn drop(&mut self) {
        // SAFETY: release exactly the reference acquired by create/load, before
        // this inner value drops its window owner. Frame clones delay this drop.
        unsafe {
            ffi::mv_image_destroy(self.native.as_ptr());
        }
    }
}
impl Frame<'_> {
    /// Queue a full image. Keep its owned data until this frame has finished.
    pub fn image(&self, image: &Image, position: (i32, i32)) -> Result<(), Error> {
        self.retain_image(image)?;
        // SAFETY: frame retention keeps texture and window live until EndDrawing.
        unsafe {
            ffi::mv_image_draw(image.inner.native.as_ptr(), position.0, position.1);
        }
        Ok(())
    }
    pub fn image_region(
        &self,
        image: &Image,
        area: Rectangle,
        transform: Transform,
    ) -> Result<(), Error> {
        if !std::ptr::eq(self.window, image.inner.window.as_ref()) {
            return Err(Error::ForeignContext);
        }
        let region = ffi::MvImageRegion {
            x: area.x,
            y: area.y,
            width: area.width,
            height: area.height,
        };
        let transform = transform.native();
        // SAFETY: immutable image and copied geometry; reject before retention.
        valid(unsafe {
            ffi::mv_image_region_valid(image.inner.native.as_ptr(), region, transform)
        })?;
        self.retain_image(image)?;
        // SAFETY: retained image, validated immutable geometry and live frame.
        valid(unsafe { ffi::mv_image_draw_region(image.inner.native.as_ptr(), region, transform) })
    }
    fn retain_image(&self, image: &Image) -> Result<(), Error> {
        if !std::ptr::eq(self.window, image.inner.window.as_ref()) {
            return Err(Error::ForeignContext);
        }
        self.images.borrow_mut().push(image.clone());
        Ok(())
    }
}
#[cfg(feature = "assets")]
impl From<crate::atlas::Region> for Rectangle {
    fn from(region: crate::atlas::Region) -> Self {
        Self {
            x: region.x,
            y: region.y,
            width: region.width,
            height: region.height,
        }
    }
}
/// Images and native cleanup stay on the window owner thread.
/// ```compile_fail
/// use malvado::image::Image;
/// fn invalid(image: Image) { std::thread::spawn(move || drop(image)); }
/// ```
mod ownership_contract {}
