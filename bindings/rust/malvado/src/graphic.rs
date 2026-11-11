// Malvado 2; SPDX-License-Identifier: Apache-2.0
//! Native simple-graphic geometry. No window or process assignment is required.
use crate::{Error, collision::Shape};
use malvado_sys as sys;
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Pivot {
    Center,
    TopLeft,
}
/// Copied immutable local geometry. Color and process ownership are separate.
#[derive(Clone, Copy, Debug)]
pub struct Geometry {
    shape: Shape,
}
impl Geometry {
    pub fn rectangle(width: f64, height: f64, pivot: Pivot) -> Result<Self, Error> {
        sys::check_abi().map_err(Error::Abi)?;
        let mut native = sys::MvCollisionShape::default();
        // SAFETY: distinct initialized output; native validates dimensions/pivot.
        let valid = unsafe {
            sys::mv_graphic_rect(
                width,
                height,
                i32::from(pivot == Pivot::TopLeft),
                &mut native,
            )
        };
        Self::checked(valid, native)
    }
    pub fn circle(radius: f64) -> Result<Self, Error> {
        sys::check_abi().map_err(Error::Abi)?;
        let mut native = sys::MvCollisionShape::default();
        // SAFETY: borrowed stack output; native retains no pointers.
        let valid = unsafe { sys::mv_graphic_circle(radius, &mut native) };
        Self::checked(valid, native)
    }
    fn checked(valid: i32, native: sys::MvCollisionShape) -> Result<Self, Error> {
        if valid != 1 {
            return Err(Error::InvalidArgument);
        }
        Ok(Self {
            shape: Shape::validated(native)?,
        })
    }
    pub fn collider(self) -> Shape {
        self.shape
    }
}

#[cfg(feature = "graphics")]
impl crate::window::Frame<'_> {
    /// Draw a copied geometry with native transform validation. Clip/scroll
    /// scopes apply. No geometry or color is retained after the call.
    pub fn graphic(
        &self,
        geometry: Geometry,
        transform: crate::collision::Transform,
        color: crate::window::Color,
    ) -> Result<(), Error> {
        // SAFETY: live owner-thread frame and copied geometry/transform values.
        crate::drawing::valid(unsafe {
            sys::graphics::mv_window_draw_graphic(geometry.shape.native, transform.native(), color)
        })
    }
}
