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
    pub(crate) native: sys::MvCamera,
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
/// use malvado::{Game, camera::{Camera, Follow}};
/// let mut game = Game::new()?;
/// let actor = game.spawn(Default::default(), |p| async move {
///     loop { p.frame().await; }
/// })?;
/// actor.set_position(100.0, 80.0)?;
/// let mut camera = Camera::new(0.0, 0.0, None)?;
/// let mut follow = Follow::new(&actor)?;
/// game.tick()?;
/// assert!(follow.update(&mut camera, (100, 80))?);
/// assert_eq!(camera.position(), (50.0, 40.0));
/// actor.kill()?;
/// game.tick()?;
/// assert!(!follow.update(&mut camera, (100, 80))?);
/// # Ok::<(), malvado::Error>(())
/// ```
///
/// This owns a handle, not the process or its resources. No scheduler is added.
/// ```compile_fail
/// # use malvado::{Game, camera::Follow};
/// let game = Game::new().unwrap();
/// let actor = game.spawn(Default::default(), |p| async move { p.frame().await; Ok(()) }).unwrap();
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

/// Context-owned camera identity.
/// ```
/// use malvado::{Game, Error, camera::{Camera, Selection}};
/// let game = Game::new()?;
/// let actor = game.spawn(Default::default(), |p| async move {loop {p.frame().await;}})?;
/// let view = game.create_camera(Camera::new(10.0, 20.0, None)?)?;
/// actor.set_camera(Selection::Explicit(view.clone()))?;
/// view.move_by(1.0, 2.0)?;
/// assert_eq!(actor.resolve_camera()?.position(), (11.0, 22.0));
/// view.close()?;
/// assert!(matches!(actor.resolve_camera(), Err(Error::NotFound)));
/// actor.set_camera(Selection::Screen)?;
/// assert_eq!(actor.resolve_camera()?.position(), (0.0, 0.0));
/// # Ok::<(), Error>(())
/// ```
/// Clones share the same camera. Dropping a
/// handle does not close it; close explicitly or drop the engine. Owner-thread only.
/// ```compile_fail
/// let game = malvado::Game::new().unwrap();
/// let camera = game.main_camera().unwrap();
/// std::thread::spawn(move || drop(camera));
/// ```
#[derive(Clone)]
pub struct CameraHandle {
    pub(crate) runtime: std::rc::Rc<crate::engine::Runtime>,
    pub(crate) reference: sys::MvCameraRef,
}
impl CameraHandle {
    pub fn id(&self) -> u64 {
        self.reference.id
    }
    pub fn snapshot(&self) -> Result<Camera, Error> {
        let mut native = sys::MvCamera::default();
        // SAFETY: checked owner-thread context, fixed identity and live output.
        result(unsafe { sys::mv_camera_get(self.runtime.live()?, self.reference, &mut native) })?;
        Ok(Camera { native })
    }
    pub fn replace(&self, value: Camera) -> Result<(), Error> {
        // SAFETY: copied camera value, checked identity and owner-thread context.
        result(unsafe { sys::mv_camera_put(self.runtime.live()?, self.reference, value.native) })
    }
    pub fn set_position(&self, x: f64, y: f64) -> Result<(), Error> {
        let mut value = self.snapshot()?;
        value.set_position(x, y)?;
        self.replace(value)
    }
    pub fn move_by(&self, dx: f64, dy: f64) -> Result<(), Error> {
        let mut value = self.snapshot()?;
        value.move_by(dx, dy)?;
        self.replace(value)
    }
    /// Idempotent explicit close. The main camera cannot be closed.
    pub fn close(&self) -> Result<(), Error> {
        // SAFETY: fixed context-bound camera identity; native validates it.
        result(unsafe { sys::mv_camera_close(self.runtime.live()?, self.reference) })
    }
}
#[derive(Clone)]
pub enum Selection {
    Main,
    Screen,
    Explicit(CameraHandle),
}
impl Handle {
    /// Copy a screen-space clipping region. None restores the camera viewport.
    /// Invalid dimensions leave the stored region unchanged.
    pub fn set_region(&self, region: Option<Viewport>) -> Result<(), Error> {
        let region = region.map(Viewport::native);
        // SAFETY: checked process identity and optional copied input; no retention.
        result(unsafe {
            sys::mv_process_region_set(
                self.inner.runtime.live()?,
                self.inner.reference,
                region.as_ref().map_or(std::ptr::null(), |r| r),
            )
        })
    }
    pub fn region(&self) -> Result<Option<Viewport>, Error> {
        let mut out = sys::MvCameraViewport::default();
        let mut present = 0;
        // SAFETY: distinct stack outputs and checked live context.
        result(unsafe {
            sys::mv_process_region_get(
                self.inner.runtime.live()?,
                self.inner.reference,
                &mut out,
                &mut present,
            )
        })?;
        Ok((present != 0).then_some(Viewport {
            x: out.x,
            y: out.y,
            width: out.width,
            height: out.height,
        }))
    }
    /// Local drawing offset, subtracted after camera projection. Does not move
    /// world coordinates or colliders. Values must be finite with absolute
    /// values at most f32::MAX/16.
    pub fn set_scroll(&self, scroll: (f32, f32)) -> Result<(), Error> {
        // SAFETY: copied scalars and checked owner-thread process identity.
        result(unsafe {
            sys::mv_process_scroll_set(
                self.inner.runtime.live()?,
                self.inner.reference,
                scroll.0,
                scroll.1,
            )
        })
    }
    pub fn scroll(&self) -> Result<(f32, f32), Error> {
        let (mut x, mut y) = (0.0, 0.0);
        // SAFETY: distinct outputs, no retention.
        result(unsafe {
            sys::mv_process_scroll_get(
                self.inner.runtime.live()?,
                self.inner.reference,
                &mut x,
                &mut y,
            )
        })?;
        Ok((x, y))
    }
    /// Intersect selected camera, region, optional host clip and logical screen.
    /// None means no visible area. A closed selected camera returns NotFound.
    pub fn view_clip(
        &self,
        host: Option<Viewport>,
        screen: (i32, i32),
    ) -> Result<Option<Viewport>, Error> {
        let host = host.map(Viewport::native);
        let mut out = sys::MvCameraViewport::default();
        // SAFETY: copied optional input and distinct output, checked context.
        result(unsafe {
            sys::mv_process_view_clip(
                self.inner.runtime.live()?,
                self.inner.reference,
                host.as_ref().map_or(std::ptr::null(), |v| v),
                screen.0,
                screen.1,
                &mut out,
            )
        })?;
        Ok((out.width > 0).then_some(Viewport {
            x: out.x,
            y: out.y,
            width: out.width,
            height: out.height,
        }))
    }
    /// Project world origin through the selected camera and local scroll.
    /// Host scroll is separate. This operation needs no window or tick.
    /// ```
    /// use malvado::{Game, camera::Viewport};
    /// let game = Game::new()?;
    /// let actor = game.spawn(Default::default(), |p| async move { loop { p.frame().await; } })?;
    /// actor.set_scroll((3.0, 4.0))?;
    /// actor.set_region(Some(Viewport { x: 10, y: 20, width: 30, height: 40 }))?;
    /// assert_eq!(actor.view_offset()?, (-3.0, -4.0));
    /// assert_eq!(actor.view_clip(None, (100, 80))?, actor.region()?);
    /// # Ok::<(), malvado::Error>(())
    /// ```
    pub fn view_offset(&self) -> Result<(f64, f64), Error> {
        let (mut x, mut y) = (0.0, 0.0);
        // SAFETY: distinct outputs, checked process and owner-thread context.
        result(unsafe {
            sys::mv_process_view_offset(
                self.inner.runtime.live()?,
                self.inner.reference,
                &mut x,
                &mut y,
            )
        })?;
        Ok((x, y))
    }
    pub fn set_camera(&self, selection: Selection) -> Result<(), Error> {
        let reference = match selection {
            Selection::Main => None,
            Selection::Screen => Some(sys::MvCameraRef::default()),
            Selection::Explicit(camera) => {
                if !std::rc::Rc::ptr_eq(&self.inner.runtime, &camera.runtime) {
                    return Err(Error::ForeignContext);
                }
                Some(camera.reference)
            }
        };
        // SAFETY: fixed process identity and copied optional camera reference.
        result(unsafe {
            sys::mv_process_camera_set(
                self.inner.runtime.live()?,
                self.inner.reference,
                reference.as_ref().map_or(std::ptr::null(), |r| r),
            )
        })
    }
    pub fn camera_selection(&self) -> Result<Selection, Error> {
        let mut reference = sys::MvCameraRef::default();
        // SAFETY: checked runtime/identity and initialized output.
        result(unsafe {
            sys::mv_process_camera_get(
                self.inner.runtime.live()?,
                self.inner.reference,
                &mut reference,
            )
        })?;
        Ok(match reference.id {
            0 => Selection::Screen,
            1 => Selection::Main,
            _ => Selection::Explicit(CameraHandle {
                runtime: self.inner.runtime.clone(),
                reference,
            }),
        })
    }
    pub fn resolve_camera(&self) -> Result<Camera, Error> {
        let mut native = sys::MvCamera::default();
        // SAFETY: checked process identity, output and live owner-thread context.
        result(unsafe {
            sys::mv_process_camera_resolve(
                self.inner.runtime.live()?,
                self.inner.reference,
                &mut native,
            )
        })?;
        Ok(Camera { native })
    }
}
impl Follow {
    /// Follow into a registered camera after the native tick. Both identities
    /// must share a context. Manual camera writes do not clear this Follow value.
    pub fn update_handle(
        &mut self,
        camera: &CameraHandle,
        window: (i32, i32),
    ) -> Result<bool, Error> {
        if let Some(target) = &self.target
            && !std::rc::Rc::ptr_eq(&target.inner.runtime, &camera.runtime)
        {
            return Err(Error::ForeignContext);
        }
        let mut value = camera.snapshot()?;
        let active = self.update(&mut value, window)?;
        camera.replace(value)?;
        Ok(active)
    }
}
