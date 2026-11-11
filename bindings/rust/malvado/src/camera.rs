// Malvado 2; SPDX-License-Identifier: Apache-2.0
//! Native camera values. Projection and clipping need no window or scheduler.
use crate::engine::result;
use crate::{Error, Handle};
use malvado_sys as sys;

/// Screen-space viewport or clipping region. Dimensions must be positive.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Viewport {
    pub x: i32,
    pub y: i32,
    pub width: i32,
    pub height: i32,
}
impl Viewport {
    fn native(self) -> sys::MvCameraViewport {
        sys::MvCameraViewport {
            x: self.x,
            y: self.y,
            width: self.width,
            height: self.height,
        }
    }
}
/// A copied native camera. None selects the full logical window.
#[derive(Clone, Copy, Debug)]
pub struct Camera {
    native: sys::MvCamera,
}
impl Camera {
    pub fn new(x: f64, y: f64, viewport: Option<Viewport>) -> Result<Self, Error> {
        sys::check_abi().map_err(Error::Abi)?;
        let mut native = sys::MvCamera::default();
        let viewport = viewport.map(Viewport::native);
        // SAFETY: output and optional viewport are live, distinct stack values.
        success(unsafe {
            sys::mv_camera_init(
                &mut native,
                x,
                y,
                viewport.as_ref().map_or(std::ptr::null(), |v| v),
            )
        })?;
        Ok(Self { native })
    }
    pub fn position(&self) -> (f64, f64) {
        (self.native.x, self.native.y)
    }
    /// Invalid coordinates leave the camera unchanged.
    pub fn set_position(&mut self, x: f64, y: f64) -> Result<(), Error> {
        // SAFETY: exclusively borrowed caller-owned native value.
        success(unsafe { sys::mv_camera_set(&mut self.native, x, y) })
    }
    pub fn move_by(&mut self, dx: f64, dy: f64) -> Result<(), Error> {
        // SAFETY: exclusively borrowed caller-owned native value.
        success(unsafe { sys::mv_camera_move(&mut self.native, dx, dy) })
    }
    /// Apply world - camera - local scroll + viewport origin.
    pub fn project(&self, world: (f64, f64), scroll: (f64, f64)) -> Result<(f64, f64), Error> {
        let (mut x, mut y) = (0.0, 0.0);
        // SAFETY: separate output pointers, no retained references.
        success(unsafe {
            sys::mv_camera_project(
                &self.native,
                world.0,
                world.1,
                scroll.0,
                scroll.1,
                &mut x,
                &mut y,
            )
        })?;
        Ok((x, y))
    }
    pub fn center(&mut self, target: (f64, f64), window: (i32, i32)) -> Result<(), Error> {
        // SAFETY: exclusive native value; C validates target and dimensions.
        success(unsafe {
            sys::mv_camera_center(&mut self.native, target.0, target.1, window.0, window.1)
        })
    }
    /// Intersect viewport, optional region and screen. None means no visible area.
    pub fn clip(
        &self,
        region: Option<Viewport>,
        screen: (i32, i32),
    ) -> Result<Option<Viewport>, Error> {
        let region = region.map(Viewport::native);
        let mut out = sys::MvCameraViewport::default();
        // SAFETY: optional input and output are distinct live values.
        match unsafe {
            sys::mv_camera_clip(
                &self.native,
                region.as_ref().map_or(std::ptr::null(), |r| r),
                screen.0,
                screen.1,
                &mut out,
            )
        } {
            0 => Ok(None),
            1 => Ok(Some(Viewport {
                x: out.x,
                y: out.y,
                width: out.width,
                height: out.height,
            })),
            _ => Err(Error::InvalidArgument),
        }
    }
}
fn success(value: i32) -> Result<(), Error> {
    if value == 1 {
        Ok(())
    } else {
        Err(Error::InvalidArgument)
    }
}

/// Context-bound follow state. Call update after the engine tick.
///
/// ```
/// use malvado::{Engine, camera::{Camera, Follow}};
/// let mut engine = Engine::new()?;
/// let actor = engine.spawn(Default::default(), |p| async move {
///     loop { p.frame().await; }
/// })?;
/// actor.set_position(100.0, 80.0)?;
/// let mut camera = Camera::new(0.0, 0.0, None)?;
/// let mut follow = Follow::new(&actor)?;
/// engine.tick()?;
/// assert!(follow.update(&mut camera, (100, 80))?);
/// assert_eq!(camera.position(), (50.0, 40.0));
/// actor.kill()?;
/// engine.tick()?;
/// assert!(!follow.update(&mut camera, (100, 80))?);
/// # Ok::<(), malvado::Error>(())
/// ```
///
/// This owns a handle, not the process or its resources. No scheduler is added.
/// ```compile_fail
/// # use malvado::{Engine, camera::Follow};
/// let engine = Engine::new().unwrap();
/// let actor = engine.spawn(Default::default(), |p| async move { p.frame().await; Ok(()) }).unwrap();
/// let follow = Follow::new(&actor).unwrap();
/// std::thread::spawn(move || drop(follow));
/// ```
pub struct Follow {
    target: Option<Handle>,
}
impl Follow {
    /// Validate a live handle without moving a camera. Failure retains no handle.
    pub fn new(target: &Handle) -> Result<Self, Error> {
        target.position()?;
        Ok(Self {
            target: Some(target.clone()),
        })
    }
    pub fn is_following(&self) -> bool {
        self.target.is_some()
    }
    /// Stop following without changing any camera position.
    pub fn clear(&mut self) {
        self.target = None;
    }
    /// Center from native position, including sleeping/frozen targets. Return
    /// false after target death; preserve final camera position and clear state.
    /// Errors preserve both camera and follow state. Does not poll a future.
    pub fn update(&mut self, camera: &mut Camera, window: (i32, i32)) -> Result<bool, Error> {
        if window.0 <= 0 || window.1 <= 0 {
            return Err(Error::InvalidArgument);
        }
        let Some(target) = &self.target else {
            return Ok(false);
        };
        let mut reference = target.inner.reference;
        // SAFETY: owner-thread live runtime, immutable context-bound identity,
        // exclusive camera and distinct copied follow reference. No retention.
        result(unsafe {
            sys::mv_camera_follow_step(
                &mut camera.native,
                target.inner.runtime.live()?,
                &mut reference,
                window.0,
                window.1,
            )
        })?;
        if reference.id == 0 {
            self.target = None;
        }
        Ok(self.is_following())
    }
}
