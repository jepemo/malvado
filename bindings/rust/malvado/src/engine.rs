// Malvado 2; SPDX-License-Identifier: Apache-2.0
use crate::resource::Resource;
use crate::task::{Frame, Step, Task, contain};
use crate::{EndReason, Error, Signal, Status};
use malvado_sys as sys;
use std::cell::Cell;
use std::ffi::c_void;
use std::future::Future;
use std::ptr::NonNull;
use std::rc::Rc;

pub(crate) fn result(code: i32) -> Result<(), Error> {
    let error = match code {
        sys::MV_OK => return Ok(()),
        sys::MV_INVALID_ARGUMENT => Error::InvalidArgument,
        sys::MV_OVERFLOW => Error::Overflow,
        sys::MV_NOT_FOUND => Error::NotFound,
        sys::MV_OUT_OF_MEMORY => Error::OutOfMemory,
        sys::MV_BUSY => Error::Busy,
        sys::MV_PROCESS_ERROR => Error::Process,
        sys::MV_QUEUE_FULL => Error::QueueFull,
        sys::MV_QUEUE_EMPTY => Error::QueueEmpty,
        sys::MV_BUFFER_TOO_SMALL => Error::BufferTooSmall,
        other => Error::UnexpectedNative(other),
    };
    Err(error)
}

pub(crate) struct Runtime {
    native: Cell<Option<NonNull<sys::MvContext>>>,
}
impl Runtime {
    pub(crate) fn live(&self) -> Result<*mut sys::MvContext, Error> {
        self.native.get().map(NonNull::as_ptr).ok_or(Error::Closed)
    }
}

/// Native scheduler owner. Dropping it cancels all remaining processes. Process
/// tokens cannot tick or destroy the engine, and may safely outlive shutdown.
pub struct Engine {
    runtime: Rc<Runtime>,
}
impl Engine {
    pub fn main_camera(&self) -> Result<crate::camera::CameraHandle, Error> {
        let mut reference = sys::MvCameraRef::default();
        // SAFETY: live owner-thread context and initialized output.
        result(unsafe { sys::mv_camera_main(self.runtime.live()?, &mut reference) })?;
        Ok(crate::camera::CameraHandle {
            runtime: self.runtime.clone(),
            reference,
        })
    }
    pub fn create_camera(
        &self,
        value: crate::camera::Camera,
    ) -> Result<crate::camera::CameraHandle, Error> {
        let mut reference = sys::MvCameraRef::default();
        // SAFETY: checked runtime and copied validated camera value. Context owns it.
        result(unsafe {
            sys::mv_camera_create(self.runtime.live()?, value.native, &mut reference)
        })?;
        Ok(crate::camera::CameraHandle {
            runtime: self.runtime.clone(),
            reference,
        })
    }

    #[cfg(feature = "graphics")]
    pub(crate) fn native_context(&self) -> Result<*mut sys::MvContext, Error> {
        self.runtime.live()
    }

    pub fn new() -> Result<Self, Error> {
        sys::check_abi().map_err(Error::Abi)?;
        // SAFETY: fixed valid native clock settings; this owner destroys once.
        let native =
            NonNull::new(unsafe { sys::mv_context_create(60.0, 5) }).ok_or(Error::OutOfMemory)?;
        Ok(Self {
            runtime: Rc::new(Runtime {
                native: Cell::new(Some(native)),
            }),
        })
    }
    pub fn spawn<F, Fut>(&self, options: SpawnOptions, run: F) -> Result<Handle, Error>
    where
        F: FnOnce(ProcessContext) -> Fut + 'static,
        Fut: Future<Output = Result<(), Error>> + 'static,
    {
        spawn(&self.runtime, 0, 0, options, run)
    }
    /// Snapshot visible process order. Sleeping processes are excluded; frozen
    /// ones remain. Later mutations do not reorder the returned handles.
    pub fn draw_order(&self) -> Result<Vec<Handle>, Error> {
        let ctx = self.runtime.live()?;
        // SAFETY: the shared helper owns the initialized output buffer.
        let records = collect_native(|out, capacity, total| unsafe {
            sys::mv_process_draw_order(ctx, out, capacity, total)
        })?;
        records
            .into_iter()
            .map(|record| acquire(&self.runtime, record.id))
            .collect()
    }
    pub fn tick(&mut self) -> Result<(), Error> {
        // SAFETY: the unique Engine borrow prevents reentrant tick/destruction.
        result(unsafe { sys::mv_process_tick(self.runtime.live()?) })
    }
    pub fn process_count(&self) -> usize {
        // SAFETY: Engine keeps the native context live on its owner thread.
        unsafe { sys::mv_process_count(self.runtime.native.get().unwrap().as_ptr()) }
    }
    pub fn signal(&self, target: &Handle, signal: Signal) -> Result<(), Error> {
        signal_in(&self.runtime, target, signal, None)
    }
    pub fn signal_tree(
        &self,
        target: &Handle,
        signal: Signal,
        relation: Relation,
    ) -> Result<(), Error> {
        signal_in(&self.runtime, target, signal, Some(relation))
    }
    pub fn find(&self, type_id: u64) -> Result<Vec<Handle>, Error> {
        let ctx = self.runtime.live()?;
        // SAFETY: copied native records and no callbacks during either read.
        let records = collect_native(|out, capacity, total| unsafe {
            sys::mv_process_find(ctx, type_id, out, capacity, total)
        })?;
        handles(&self.runtime, records)
    }
    pub fn lineage(&self) -> Result<Vec<LineageEntry>, Error> {
        let ctx = self.runtime.live()?;
        // SAFETY: initialized output storage and no native callbacks.
        let records = collect_native(|out, capacity, total| unsafe {
            sys::mv_process_lineage_snapshot(ctx, out, capacity, total)
        })?;
        Ok(records
            .into_iter()
            .map(|r| LineageEntry {
                id: r.id,
                creator_id: r.creator_id,
                alive: r.alive != 0,
            })
            .collect())
    }
    pub fn resource<T: 'static>(&self, value: T) -> Result<Resource<T>, Error> {
        Resource::create(&self.runtime, None, value)
    }
}
impl Drop for Engine {
    fn drop(&mut self) {
        // Invalidate tokens before destructors run; none can reenter shutdown.
        if let Some(native) = self.runtime.native.take() {
            // SAFETY: Engine is not Send and cannot be dropped during its tick.
            unsafe {
                sys::mv_context_destroy(native.as_ptr());
            }
        }
    }
}

/// Owned process configuration. Child spawns attach by default. Root spawns
/// have no creator or owner. FPS 0/60 means every eligible tick.
#[derive(Clone, Copy, Debug)]
pub struct SpawnOptions {
    pub priority: i64,
    pub fps: u32,
    pub attached: bool,
    pub type_id: u64,
}
impl Default for SpawnOptions {
    fn default() -> Self {
        Self {
            priority: 0,
            fps: 0,
            attached: true,
            type_id: 0,
        }
    }
}

/// Copyable native state, never a borrow into suspended Rust game state.
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Snapshot {
    pub alive: bool,
    pub status: Option<Status>,
    pub reason: Option<EndReason>,
    pub creator_id: u64,
    pub owner_id: u64,
    pub priority: i64,
    pub type_id: u64,
}
pub(crate) struct HandleInner {
    native: NonNull<sys::MvProcessHandle>,
    pub(crate) runtime: Rc<Runtime>,
    pub(crate) reference: sys::MvProcessRef,
}
impl Drop for HandleInner {
    fn drop(&mut self) {
        // SAFETY: owned handles can be released after their native context ends.
        unsafe {
            sys::mv_process_handle_destroy(self.native.as_ptr());
        }
    }
}
#[derive(Clone)]
pub struct Handle {
    pub(crate) inner: Rc<HandleInner>,
}
impl PartialEq for Handle {
    fn eq(&self, other: &Self) -> bool {
        self.inner.reference == other.inner.reference
    }
}
impl Eq for Handle {}
impl Handle {
    pub fn id(&self) -> u64 {
        self.inner.reference.id
    }
    pub fn snapshot(&self) -> Result<Snapshot, Error> {
        let mut info = sys::MvProcessHandleInfo::default();
        // SAFETY: this owned handle and output remain valid for the whole call.
        result(unsafe { sys::mv_process_handle_snapshot(self.inner.native.as_ptr(), &mut info) })?;
        let (status, reason) = if info.alive != 0 {
            let status = match info.process.status {
                sys::MV_PROCESS_RUNNING => Status::Running,
                sys::MV_PROCESS_SLEEPING => Status::Sleeping,
                sys::MV_PROCESS_FROZEN => Status::Frozen,
                _ => return Err(Error::UnexpectedNative(info.process.status)),
            };
            (Some(status), None)
        } else {
            let reason = match info.reason {
                sys::MV_END_KILLED => EndReason::Killed,
                sys::MV_END_RETURNED => EndReason::Returned,
                sys::MV_END_ERROR => EndReason::Error,
                _ => return Err(Error::UnexpectedNative(info.reason)),
            };
            (None, Some(reason))
        };
        Ok(Snapshot {
            alive: info.alive != 0,
            status,
            reason,
            creator_id: info.process.creator_id,
            owner_id: info.process.parent,
            priority: info.process.priority,
            type_id: info.process.type_id,
        })
    }
    pub fn children(&self, relation: Relation, type_id: u64) -> Result<Vec<Handle>, Error> {
        self.related(relation, type_id, false)
    }
    pub fn descendants(&self, relation: Relation, type_id: u64) -> Result<Vec<Handle>, Error> {
        self.related(relation, type_id, true)
    }
    fn related(
        &self,
        relation: Relation,
        type_id: u64,
        recursive: bool,
    ) -> Result<Vec<Handle>, Error> {
        let ctx = self.inner.runtime.live()?;
        let read = match (relation, recursive) {
            (Relation::Ownership, false) => sys::mv_process_children,
            (Relation::Ownership, true) => sys::mv_process_descendants,
            (Relation::Creation, false) => sys::mv_process_creation_children,
            (Relation::Creation, true) => sys::mv_process_creation_descendants,
        };
        // SAFETY: the handle fixes the runtime; query copies initialized records.
        let records = collect_native(|out, capacity, total| unsafe {
            read(ctx, self.id(), type_id, out, capacity, total)
        })?;
        handles(&self.inner.runtime, records)
    }
    pub fn kill(&self) -> Result<bool, Error> {
        let mut killed = 0;
        // SAFETY: immutable context-scoped identity, valid owner-thread context.
        result(unsafe {
            sys::mv_process_kill(
                self.inner.runtime.live()?,
                self.inner.reference,
                &mut killed,
            )
        })?;
        Ok(killed != 0)
    }
    /// Copy native position. Lua numeric fields and graph rendering share this state.
    pub fn position(&self) -> Result<(f64, f64), Error> {
        let (mut x, mut y) = (0.0, 0.0);
        // SAFETY: live owner-thread context, fixed identity and distinct outputs.
        result(unsafe {
            sys::mv_process_position(
                self.inner.runtime.live()?,
                self.inner.reference,
                &mut x,
                &mut y,
            )
        })?;
        Ok((x, y))
    }
    /// Set both finite native coordinates atomically, without updating colliders.
    pub fn set_position(&self, x: f64, y: f64) -> Result<(), Error> {
        // SAFETY: live owner-thread context; native validates identity and coordinates.
        result(unsafe {
            sys::mv_process_set_position(self.inner.runtime.live()?, self.inner.reference, x, y)
        })
    }
    /// Copy native visual angle. Collider transforms remain explicit in Rust.
    pub fn angle(&self) -> Result<f64, Error> {
        let mut value = 0.0;
        // SAFETY: live owner-thread context, fixed identity and valid output.
        result(unsafe {
            sys::mv_process_angle(self.inner.runtime.live()?, self.inner.reference, &mut value)
        })?;
        Ok(value)
    }
    /// Store finite angle without normalization. Does not update colliders.
    pub fn set_angle(&self, value: f64) -> Result<(), Error> {
        // SAFETY: live owner-thread context; native validates identity and value.
        result(unsafe {
            sys::mv_process_set_angle(self.inner.runtime.live()?, self.inner.reference, value)
        })
    }
    /// Copy native visual scale. Collider transforms remain explicit in Rust.
    pub fn scale(&self) -> Result<f64, Error> {
        let mut value = 0.0;
        // SAFETY: live owner-thread context, fixed identity and valid output.
        result(unsafe {
            sys::mv_process_scale(self.inner.runtime.live()?, self.inner.reference, &mut value)
        })?;
        Ok(value)
    }
    /// Store finite scale without normalization. Does not update colliders.
    pub fn set_scale(&self, value: f64) -> Result<(), Error> {
        // SAFETY: live owner-thread context; native validates identity and value.
        result(unsafe {
            sys::mv_process_set_scale(self.inner.runtime.live()?, self.inner.reference, value)
        })
    }
    pub fn z(&self) -> Result<f64, Error> {
        let mut z = 0.0;
        // SAFETY: checked owner-thread runtime, immutable identity and output.
        result(unsafe {
            sys::mv_process_z(self.inner.runtime.live()?, self.inner.reference, &mut z)
        })?;
        Ok(z)
    }
    pub fn set_z(&self, z: f64) -> Result<(), Error> {
        // SAFETY: live owner-thread context; native validates identity and z.
        result(unsafe {
            sys::mv_process_set_z(self.inner.runtime.live()?, self.inner.reference, z)
        })
    }
    pub fn set_priority(&self, priority: i64) -> Result<(), Error> {
        // SAFETY: handle fixes the context; the C API validates liveness.
        result(unsafe {
            sys::mv_process_set_priority(self.inner.runtime.live()?, self.id(), priority)
        })
    }
    pub fn set_type(&self, type_id: u64) -> Result<(), Error> {
        // SAFETY: handle fixes the context; the C API validates liveness.
        result(unsafe { sys::mv_process_set_type(self.inner.runtime.live()?, self.id(), type_id) })
    }
}

#[derive(Clone)]
pub struct ProcessContext {
    current: Handle,
}
impl ProcessContext {
    pub fn current(&self) -> Handle {
        self.current.clone()
    }
    pub fn frame(&self) -> Frame {
        Frame::new()
    }
    pub fn spawn<F, Fut>(&self, options: SpawnOptions, run: F) -> Result<Handle, Error>
    where
        F: FnOnce(ProcessContext) -> Fut + 'static,
        Fut: Future<Output = Result<(), Error>> + 'static,
    {
        let owner = if options.attached {
            self.current.id()
        } else {
            0
        };
        spawn(
            &self.current.inner.runtime,
            self.current.id(),
            owner,
            options,
            run,
        )
    }
    pub fn signal(&self, target: &Handle, signal: Signal) -> Result<(), Error> {
        signal_in(&self.current.inner.runtime, target, signal, None)
    }
    pub fn signal_tree(
        &self,
        target: &Handle,
        signal: Signal,
        relation: Relation,
    ) -> Result<(), Error> {
        signal_in(&self.current.inner.runtime, target, signal, Some(relation))
    }
    pub fn resource<T: 'static>(&self, value: T) -> Result<Resource<T>, Error> {
        Resource::create(&self.current.inner.runtime, Some(&self.current), value)
    }
}
fn signal_in(
    runtime: &Rc<Runtime>,
    target: &Handle,
    signal: Signal,
    tree: Option<Relation>,
) -> Result<(), Error> {
    if !Rc::ptr_eq(runtime, &target.inner.runtime) {
        return Err(Error::ForeignContext);
    }
    let signal = match signal {
        Signal::Sleep => sys::MV_SIGNAL_SLEEP,
        Signal::Freeze => sys::MV_SIGNAL_FREEZE,
        Signal::Wake => sys::MV_SIGNAL_WAKEUP,
        Signal::Kill => sys::MV_SIGNAL_KILL,
    };
    // SAFETY: the context comparison prevents foreign local ID reuse.
    result(unsafe {
        if tree == Some(Relation::Creation) {
            sys::mv_process_signal_creation_tree(runtime.live()?, target.id(), signal)
        } else {
            sys::mv_process_signal(
                runtime.live()?,
                target.id(),
                signal,
                i32::from(tree.is_some()),
            )
        }
    })
}

struct NativeTask {
    task: Task,
    cadence: sys::MvCadence,
    fps: u32,
    first: bool,
}
unsafe extern "C" fn step(_: *mut sys::MvContext, _: u64, user: *mut c_void) -> i32 {
    let outcome = contain(|| {
        // SAFETY: successful spawn gives C this Box until safe-point cleanup.
        let process = unsafe { &mut *user.cast::<NativeTask>() };
        if process.first {
            process.first = false;
        }
        // SAFETY: validated fps and exclusively callback-owned cadence state.
        else if unsafe { sys::mv_cadence_step(&mut process.cadence, process.fps) } == 0 {
            return Step::Yield;
        }
        process.task.poll()
    });
    match outcome {
        Ok(Step::Yield) => sys::MV_STEP_YIELD,
        Ok(Step::Done) => sys::MV_STEP_DONE,
        _ => sys::MV_STEP_ERROR,
    }
}
unsafe extern "C" fn destroy(user: *mut c_void) {
    // SAFETY: C invokes this exactly once, after poll has returned.
    let _ = contain(|| unsafe {
        drop(Box::from_raw(user.cast::<NativeTask>()));
    });
}
fn spawn<F, Fut>(
    runtime: &Rc<Runtime>,
    creator: u64,
    owner: u64,
    options: SpawnOptions,
    run: F,
) -> Result<Handle, Error>
where
    F: FnOnce(ProcessContext) -> Fut + 'static,
    Fut: Future<Output = Result<(), Error>> + 'static,
{
    if options.fps > 60 {
        return Err(Error::InvalidArgument);
    }
    let ctx = runtime.live()?;
    let task = Box::new(NativeTask {
        task: Task::empty(),
        cadence: sys::MvCadence::default(),
        fps: options.fps,
        first: true,
    });
    let user = Box::into_raw(task);
    let mut id = 0;
    // SAFETY: the allocation remains stable; spawn never invokes the callbacks.
    let code = unsafe {
        sys::mv_process_spawn_related(
            ctx,
            creator,
            owner,
            options.priority,
            Some(step),
            Some(destroy),
            user.cast(),
            &mut id,
        )
    };
    if let Err(error) = result(code) {
        // SAFETY: failed spawn never takes responsibility for user.
        unsafe {
            drop(Box::from_raw(user));
        }
        return Err(error);
    }
    let mut reference = sys::MvProcessRef::default();
    let mut native = std::ptr::null_mut();
    // SAFETY: newly spawned ID is live; all output storage is valid.
    let setup = unsafe {
        result(sys::mv_process_ref_create(ctx, id, &mut reference))
            .and_then(|()| result(sys::mv_process_handle_create(ctx, reference, &mut native)))
            .and_then(|()| result(sys::mv_process_set_type(ctx, id, options.type_id)))
    };
    if let Err(error) = setup {
        // SAFETY: cleanup is deferred by the same native scheduler. No future
        // was installed, and the caller's factory is dropped on this error.
        unsafe {
            sys::mv_process_signal(ctx, id, sys::MV_SIGNAL_KILL, 0);
            if !native.is_null() {
                sys::mv_process_handle_destroy(native);
            }
        }
        return Err(error);
    }
    let handle = Handle {
        inner: Rc::new(HandleInner {
            native: NonNull::new(native).unwrap(),
            runtime: runtime.clone(),
            reference,
        }),
    };
    let process = ProcessContext {
        current: handle.clone(),
    };
    // SAFETY: spawn cannot run until the next tick, so C has no active borrow.
    // Lazy construction puts factory panics inside the poll panic boundary.
    unsafe {
        (*user).task.install(async move { run(process).await });
    }
    Ok(handle)
}

/// Select the native ownership or historical creation relation explicitly.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Relation {
    Ownership,
    Creation,
}

/// A copied retained creation node. IDs belong to the engine returning it.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct LineageEntry {
    pub id: u64,
    pub creator_id: u64,
    pub alive: bool,
}

pub(crate) fn collect_native<T: Default + Clone>(
    mut read: impl FnMut(*mut T, usize, *mut usize) -> i32,
) -> Result<Vec<T>, Error> {
    let mut total = 0;
    result(read(std::ptr::null_mut(), 0, &mut total))?;
    let mut records = Vec::new();
    records
        .try_reserve_exact(total)
        .map_err(|_| Error::OutOfMemory)?;
    records.resize(total, T::default());
    result(read(records.as_mut_ptr(), records.len(), &mut total))?;
    if total > records.len() {
        return Err(Error::BufferTooSmall);
    }
    records.truncate(total);
    Ok(records)
}

fn handles(runtime: &Rc<Runtime>, records: Vec<sys::MvProcessInfo>) -> Result<Vec<Handle>, Error> {
    records
        .into_iter()
        .map(|record| acquire(runtime, record.id))
        .collect()
}

pub(crate) fn acquire(runtime: &Rc<Runtime>, id: u64) -> Result<Handle, Error> {
    let ctx = runtime.live()?;
    let mut reference = sys::MvProcessRef::default();
    let mut native = std::ptr::null_mut();
    // SAFETY: the native APIs validate the ID and copy/allocate owned outputs.
    unsafe {
        result(sys::mv_process_ref_create(ctx, id, &mut reference))?;
        result(sys::mv_process_handle_create(ctx, reference, &mut native))?;
    }
    Ok(Handle {
        inner: Rc::new(HandleInner {
            native: NonNull::new(native).expect("successful owned handle allocation"),
            runtime: runtime.clone(),
            reference,
        }),
    })
}

#[cfg(test)]
mod query_tests {
    use super::*;

    #[test]
    fn copied_query_buffers_handle_empty_shrinking_and_growing_counts() {
        for (needed, actual) in [(0, 0), (4, 3), (2, 3)] {
            let result = collect_native::<u64>(|out, capacity, total| {
                // SAFETY: collect_native supplies one count slot and capacity
                // initialized output slots. Never write beyond that capacity.
                unsafe {
                    *total = if capacity == 0 { needed } else { actual };
                    for index in 0..capacity.min(actual) {
                        *out.add(index) = index as u64;
                    }
                }
                sys::MV_OK
            });
            if actual > needed {
                assert_eq!(result, Err(Error::BufferTooSmall));
            } else {
                assert_eq!(result.unwrap(), (0..actual as u64).collect::<Vec<_>>());
            }
        }
        assert_eq!(
            collect_native::<u64>(|_, _, _| sys::MV_NOT_FOUND),
            Err(Error::NotFound)
        );
    }
}
