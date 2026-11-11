// Malvado 2; SPDX-License-Identifier: Apache-2.0
//! Native collision math and owned static worlds. These explicit shape/value
//! operations do not yet derive graphics or mutate process visual properties.
use crate::engine::{acquire, collect_native, result};
use crate::{Error, Handle};
use malvado_sys as sys;
use std::marker::PhantomData;
use std::ptr::NonNull;
use std::rc::Rc;

#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Rect {
    pub x: f64,
    pub y: f64,
    pub width: f64,
    pub height: f64,
}
impl Rect {
    fn native(self) -> sys::MvRect {
        sys::MvRect {
            x: self.x,
            y: self.y,
            width: self.width,
            height: self.height,
        }
    }
    pub fn overlaps(self, other: Self) -> Result<bool, Error> {
        sys::check_abi().map_err(Error::Abi)?;
        // SAFETY: copied values only; native math validates geometry.
        predicate(unsafe { sys::mv_rect_overlaps(self.native(), other.native()) })
    }
    pub fn contains(self, x: f64, y: f64) -> Result<bool, Error> {
        sys::check_abi().map_err(Error::Abi)?;
        // SAFETY: copied values only; no native ownership or context.
        predicate(unsafe { sys::mv_rect_contains(self.native(), x, y) })
    }
}

#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Transform {
    pub x: f64,
    pub y: f64,
    pub angle: f64,
    pub scale: f64,
}
impl Default for Transform {
    fn default() -> Self {
        Self {
            x: 0.0,
            y: 0.0,
            angle: 0.0,
            scale: 1.0,
        }
    }
}
impl Transform {
    fn native(self) -> sys::MvCollisionTransform {
        sys::MvCollisionTransform {
            x: self.x,
            y: self.y,
            angle: self.angle,
            scale: self.scale,
        }
    }
}

#[derive(Clone, Copy, Debug)]
pub struct Shape {
    native: sys::MvCollisionShape,
}
impl Shape {
    pub fn circle(radius: f64) -> Result<Self, Error> {
        Self::validated(sys::MvCollisionShape {
            kind: sys::MV_COLLISION_CIRCLE,
            radius,
            ..Default::default()
        })
    }
    pub fn rectangle(width: f64, height: f64) -> Result<Self, Error> {
        Self::validated(sys::MvCollisionShape {
            kind: sys::MV_COLLISION_RECT,
            width,
            height,
            ..Default::default()
        })
    }
    pub fn with_offset(self, x: f64, y: f64) -> Result<Self, Error> {
        Self::validated(sys::MvCollisionShape {
            offset_x: x,
            offset_y: y,
            ..self.native
        })
    }
    pub(crate) fn validated(native: sys::MvCollisionShape) -> Result<Self, Error> {
        sys::check_abi().map_err(Error::Abi)?;
        let shape = Self { native };
        shape.overlaps(Transform::default(), shape, Transform::default())?;
        Ok(shape)
    }
    pub fn overlaps(
        self,
        transform: Transform,
        other: Self,
        other_transform: Transform,
    ) -> Result<bool, Error> {
        // SAFETY: a Shape can only be built after ABI validation; native math
        // validates transforms and uses the copied values without retention.
        predicate(unsafe {
            sys::mv_collision_overlap(
                self.native,
                transform.native(),
                other.native,
                other_transform.native(),
            )
        })
    }
}
fn predicate(status: i32) -> Result<bool, Error> {
    match status {
        0 => Ok(false),
        1 => Ok(true),
        _ => Err(Error::InvalidArgument),
    }
}

#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Sweep {
    pub hit: Option<usize>,
    pub fraction: f64,
    pub normal_x: f64,
    pub normal_y: f64,
}
impl From<sys::MvCollisionSweep> for Sweep {
    fn from(value: sys::MvCollisionSweep) -> Self {
        Self {
            hit: if value.hit == 0 { None } else { Some(value.id) },
            fraction: value.fraction,
            normal_x: value.normal_x,
            normal_y: value.normal_y,
        }
    }
}

/// Caller-owned native world. No process/graphics state is duplicated here.
/// Methods and Drop remain on the owner thread. World geometry owns its data
/// and can outlive an engine; it does not retain any engine or platform service.
pub struct World {
    native: NonNull<sys::MvCollisionWorld>,
    owner_thread: PhantomData<Rc<()>>,
}
impl World {
    pub fn new() -> Result<Self, Error> {
        sys::check_abi().map_err(Error::Abi)?;
        // SAFETY: native create takes no input and returns caller ownership.
        let native =
            NonNull::new(unsafe { sys::mv_collision_world_create() }).ok_or(Error::OutOfMemory)?;
        Ok(Self {
            native,
            owner_thread: PhantomData,
        })
    }
    pub fn add_rect(&mut self, rect: Rect) -> Result<usize, Error> {
        let mut id = 0;
        // SAFETY: exclusive owned world; native copies and validates the rect.
        if unsafe { sys::mv_collision_world_add_rect(self.native.as_ptr(), rect.native(), &mut id) }
            == 1
        {
            Ok(id)
        } else {
            Err(Error::WorldInsert)
        } // C combines invalid input, OOM and limit.
    }
    pub fn query(&self, shape: Shape, transform: Transform) -> Result<Option<usize>, Error> {
        let mut id = 0;
        // SAFETY: borrowed live world and copied validated shape.
        let hit = predicate(unsafe {
            sys::mv_collision_world_query(
                self.native.as_ptr(),
                shape.native,
                transform.native(),
                &mut id,
            )
        })?;
        Ok(if hit { Some(id) } else { None })
    }
    pub fn sweep(
        &self,
        shape: Shape,
        transform: Transform,
        dx: f64,
        dy: f64,
    ) -> Result<Sweep, Error> {
        let mut out = sys::MvCollisionSweep::default();
        // SAFETY: borrowed world, copied values and initialized output.
        if unsafe {
            sys::mv_collision_world_sweep(
                self.native.as_ptr(),
                shape.native,
                transform.native(),
                dx,
                dy,
                &mut out,
            )
        } != 1
        {
            return Err(Error::InvalidArgument);
        }
        Ok(out.into())
    }
    pub fn move_by(
        &self,
        shape: Shape,
        transform: &mut Transform,
        dx: f64,
        dy: f64,
    ) -> Result<Sweep, Error> {
        let mut native = transform.native();
        let mut out = sys::MvCollisionSweep::default();
        // SAFETY: native writes only these temporary outputs; commit on success.
        if unsafe {
            sys::mv_collision_world_move(
                self.native.as_ptr(),
                shape.native,
                &mut native,
                dx,
                dy,
                &mut out,
            )
        } != 1
        {
            return Err(Error::InvalidArgument);
        }
        *transform = Transform {
            x: native.x,
            y: native.y,
            angle: native.angle,
            scale: native.scale,
        };
        Ok(out.into())
    }
}
impl Drop for World {
    fn drop(&mut self) {
        // SAFETY: unique native world ownership is released exactly once.
        unsafe {
            sys::mv_collision_world_destroy(self.native.as_ptr());
        }
    }
}

impl Handle {
    pub fn set_collider(&self, shape: Shape, transform: Transform) -> Result<(), Error> {
        // SAFETY: context-bound live identity; native copies validated values.
        result(unsafe {
            sys::mv_process_collider_set(
                self.inner.runtime.live()?,
                self.inner.reference,
                shape.native,
                transform.native(),
            )
        })
    }
    pub fn clear_collider(&self) -> Result<(), Error> {
        // SAFETY: context bound identity, checked by native code.
        result(unsafe {
            sys::mv_process_collider_clear(self.inner.runtime.live()?, self.inner.reference)
        })
    }
    pub fn collides_with(&self, other: &Handle) -> Result<bool, Error> {
        if !Rc::ptr_eq(&self.inner.runtime, &other.inner.runtime) {
            return Err(Error::ForeignContext);
        }
        let mut overlap = 0;
        // SAFETY: both identities share a live context and output is initialized.
        result(unsafe {
            sys::mv_process_collider_test(
                self.inner.runtime.live()?,
                self.inner.reference,
                other.inner.reference,
                &mut overlap,
            )
        })?;
        Ok(overlap != 0)
    }
    pub fn first_collision(&self, type_id: u64) -> Result<Option<Handle>, Error> {
        let mut reference = sys::MvProcessRef::default();
        // SAFETY: fixed context and initialized copied identity output.
        result(unsafe {
            sys::mv_process_collider_first(
                self.inner.runtime.live()?,
                self.inner.reference,
                type_id,
                &mut reference,
            )
        })?;
        if reference.id == 0 {
            Ok(None)
        } else {
            acquire(&self.inner.runtime, reference.id).map(Some)
        }
    }
    pub fn collisions(&self, type_id: u64) -> Result<Vec<Handle>, Error> {
        let ctx = self.inner.runtime.live()?;
        // SAFETY: no callbacks or native mutations between reads; initialized
        // copied references remain local to the source's context.
        let references = collect_native(|out, capacity, total| unsafe {
            sys::mv_process_collider_find(ctx, self.inner.reference, type_id, out, capacity, total)
        })?;
        references
            .into_iter()
            .map(|r| acquire(&self.inner.runtime, r.id))
            .collect()
    }
}

/// World mutation and destruction remain on the owner thread.
/// ```compile_fail
/// let world = malvado::collision::World::new().unwrap();
/// std::thread::spawn(move || drop(world));
/// ```
mod ownership_contract {}
