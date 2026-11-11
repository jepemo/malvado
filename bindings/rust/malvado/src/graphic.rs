// Malvado 2; SPDX-License-Identifier: Apache-2.0
//! Native primitive geometry and copied process graph values.
use crate::engine::result;
use crate::{Error, Handle, collision::Shape};
use malvado_sys as sys;
pub use malvado_sys::MvColor as Color;
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

/// Copied primitive graph. Assignment retains no external resource.
/// ```
/// use malvado::{Engine, graphic::{Graphic, Geometry, Pivot, Color}};
/// let engine = Engine::new()?;
/// let actor = engine.spawn(Default::default(), |p| async move {
///     loop { p.frame().await; }
/// })?;
/// actor.set_graphic(Graphic {
///     geometry: Geometry::rectangle(20.0, 10.0, Pivot::Center)?,
///     color: Color { r: 255, g: 0, b: 0, a: 255 },
/// })?;
/// assert_eq!(actor.graphic()?.unwrap().color.r, 255);
/// actor.clear_graphic()?;
/// assert!(actor.graphic()?.is_none());
/// # Ok::<(), malvado::Error>(())
/// ```
#[derive(Clone, Copy, Debug)]
pub struct Graphic {
    pub geometry: Geometry,
    pub color: Color,
}
impl Handle {
    pub fn set_graphic(&self, graphic: Graphic) -> Result<(), Error> {
        // SAFETY: live owner-thread context and copied validated geometry/color.
        result(unsafe {
            sys::mv_process_graphic_set(
                self.inner.runtime.live()?,
                self.inner.reference,
                sys::MvGraphic {
                    shape: graphic.geometry.shape.native,
                    color: graphic.color,
                },
            )
        })
    }
    pub fn graphic(&self) -> Result<Option<Graphic>, Error> {
        let mut native = sys::MvGraphic::default();
        let mut present = 0;
        // SAFETY: live context and separate initialized outputs; no retention.
        result(unsafe {
            sys::mv_process_graphic_get(
                self.inner.runtime.live()?,
                self.inner.reference,
                &mut native,
                &mut present,
            )
        })?;
        if present == 0 {
            return Ok(None);
        }
        Ok(Some(Graphic {
            geometry: Geometry::checked(1, native.shape)?,
            color: native.color,
        }))
    }
    pub fn clear_graphic(&self) -> Result<(), Error> {
        // SAFETY: fixed identity and live owner-thread runtime.
        result(unsafe {
            sys::mv_process_graphic_clear(self.inner.runtime.live()?, self.inner.reference)
        })
    }
}
#[cfg(feature = "graphics")]
impl crate::window::Frame<'_> {
    /// Draw stored primitive and image process graphs in native visual order. This explicit
    /// pass uses registered camera selection and restores host clip/scroll scopes.
    /// It does not tick, collect commands or advance animations.
    pub fn process_graphics(&self, engine: &crate::Engine) -> Result<(), Error> {
        // SAFETY: frame-bound call and borrowed live engine on its owner thread.
        crate::drawing::valid(unsafe {
            sys::graphics::mv_window_draw_process_graphics(engine.native_context()?)
        })
    }
}
