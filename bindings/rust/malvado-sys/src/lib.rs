// Malvado 2; SPDX-License-Identifier: Apache-2.0
//! Unsafe native bindings. Call `check_abi` before any other engine operation.
//! All pointer, ownership, callback and owner-thread rules in the installed C
//! headers apply. This crate does not make native operations safe.
#![allow(non_snake_case)]
use std::ffi::{CStr, c_char, c_int, c_void};

pub const MV_ABI_VERSION: u32 = 2;
pub type MvProcessId = u64;
#[repr(C)]
pub struct MvContext {
    _private: [u8; 0],
}
#[repr(C)]
pub struct MvProcessHandle {
    _private: [u8; 0],
}
#[repr(C)]
pub struct MvResource {
    _private: [u8; 0],
}
#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct MvCadence {
    pub phase: u32,
}
#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct MvRuntimeId {
    pub high: u64,
    pub low: u64,
    pub sequence: u64,
}
#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct MvProcessRef {
    pub runtime: MvRuntimeId,
    pub id: MvProcessId,
}
#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct MvProcessInfo {
    pub id: MvProcessId,
    pub parent: MvProcessId,
    pub type_id: u64,
    pub priority: i64,
    pub status: c_int,
    pub messages: usize,
    pub creator_id: MvProcessId,
    pub attached: c_int,
}
#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct MvProcessHandleInfo {
    pub reference: MvProcessRef,
    pub alive: c_int,
    pub process: MvProcessInfo,
    pub reason: c_int,
}
#[repr(C)]
#[derive(Clone, Copy, Debug, Default)]
pub struct MvLineageInfo {
    pub id: u64,
    pub creator_id: u64,
    pub alive: c_int,
}
#[repr(C)]
pub struct MvCollisionWorld {
    _private: [u8; 0],
}
pub const MV_COLLISION_CIRCLE: c_int = 0;
pub const MV_COLLISION_RECT: c_int = 1;
#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub struct MvRect {
    pub x: f64,
    pub y: f64,
    pub width: f64,
    pub height: f64,
}
#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub struct MvCollisionShape {
    pub kind: c_int,
    pub width: f64,
    pub height: f64,
    pub radius: f64,
    pub offset_x: f64,
    pub offset_y: f64,
}
#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub struct MvCollisionTransform {
    pub x: f64,
    pub y: f64,
    pub angle: f64,
    pub scale: f64,
}
#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub struct MvCollisionSweep {
    pub hit: c_int,
    pub id: usize,
    pub fraction: f64,
    pub normal_x: f64,
    pub normal_y: f64,
}
pub type MvProcessStep = Option<unsafe extern "C" fn(*mut MvContext, u64, *mut c_void) -> c_int>;
pub type MvProcessDestroy = Option<unsafe extern "C" fn(*mut c_void)>;
pub type MvResourceDestroy = Option<unsafe extern "C" fn(*mut c_void)>;
pub const MV_OK: c_int = 0;
pub const MV_INVALID_ARGUMENT: c_int = 1;
pub const MV_OVERFLOW: c_int = 2;
pub const MV_NOT_FOUND: c_int = 3;
pub const MV_OUT_OF_MEMORY: c_int = 4;
pub const MV_BUSY: c_int = 5;
pub const MV_PROCESS_ERROR: c_int = 6;
pub const MV_QUEUE_FULL: c_int = 7;
pub const MV_QUEUE_EMPTY: c_int = 8;
pub const MV_BUFFER_TOO_SMALL: c_int = 9;
pub const MV_SIGNAL_SLEEP: c_int = 0;
pub const MV_SIGNAL_FREEZE: c_int = 1;
pub const MV_SIGNAL_WAKEUP: c_int = 2;
pub const MV_SIGNAL_KILL: c_int = 3;
pub const MV_STEP_YIELD: c_int = 0;
pub const MV_STEP_DONE: c_int = 1;
pub const MV_STEP_ERROR: c_int = 2;
pub const MV_PROCESS_RUNNING: c_int = 0;
pub const MV_PROCESS_SLEEPING: c_int = 1;
pub const MV_PROCESS_FROZEN: c_int = 2;
pub const MV_END_KILLED: c_int = 0;
pub const MV_END_RETURNED: c_int = 1;
pub const MV_END_ERROR: c_int = 2;
unsafe extern "C" {
    pub fn mv_rect_overlaps(a: MvRect, b: MvRect) -> c_int;
    pub fn mv_rect_contains(rect: MvRect, x: f64, y: f64) -> c_int;
    pub fn mv_collision_overlap(
        a: MvCollisionShape,
        at: MvCollisionTransform,
        b: MvCollisionShape,
        bt: MvCollisionTransform,
    ) -> c_int;
    pub fn mv_process_collider_bind(
        ctx: *mut MvContext,
        process: MvProcessRef,
        shape: *const MvCollisionShape,
    ) -> c_int;
    pub fn mv_process_collider_set(
        ctx: *mut MvContext,
        process: MvProcessRef,
        shape: MvCollisionShape,
        transform: MvCollisionTransform,
    ) -> c_int;
    pub fn mv_process_collider_clear(ctx: *mut MvContext, process: MvProcessRef) -> c_int;
    pub fn mv_process_collider_test(
        ctx: *mut MvContext,
        a: MvProcessRef,
        b: MvProcessRef,
        overlap: *mut c_int,
    ) -> c_int;
    pub fn mv_process_collider_first(
        ctx: *mut MvContext,
        source: MvProcessRef,
        type_id: u64,
        out: *mut MvProcessRef,
    ) -> c_int;
    pub fn mv_process_collider_find(
        ctx: *mut MvContext,
        source: MvProcessRef,
        type_id: u64,
        out: *mut MvProcessRef,
        capacity: usize,
        total: *mut usize,
    ) -> c_int;
    pub fn mv_collision_world_create() -> *mut MvCollisionWorld;
    pub fn mv_collision_world_destroy(world: *mut MvCollisionWorld) -> ();
    pub fn mv_collision_world_add_rect(
        world: *mut MvCollisionWorld,
        rect: MvRect,
        id: *mut usize,
    ) -> c_int;
    pub fn mv_collision_world_query(
        world: *const MvCollisionWorld,
        shape: MvCollisionShape,
        transform: MvCollisionTransform,
        id: *mut usize,
    ) -> c_int;
    pub fn mv_collision_world_sweep(
        world: *const MvCollisionWorld,
        shape: MvCollisionShape,
        transform: MvCollisionTransform,
        dx: f64,
        dy: f64,
        out: *mut MvCollisionSweep,
    ) -> c_int;
    pub fn mv_process_world_query_selected(
        ctx: *mut MvContext,
        process: MvProcessRef,
        world: *const MvCollisionWorld,
        out: *mut usize,
    ) -> c_int;
    pub fn mv_process_world_move_selected(
        ctx: *mut MvContext,
        process: MvProcessRef,
        world: *const MvCollisionWorld,
        dx: f64,
        dy: f64,
        out: *mut MvCollisionSweep,
    ) -> c_int;
    pub fn mv_process_world_query(
        ctx: *mut MvContext,
        process: MvProcessRef,
        world: *const MvCollisionWorld,
        shape: *const MvCollisionShape,
        out: *mut usize,
    ) -> c_int;
    pub fn mv_process_world_move(
        ctx: *mut MvContext,
        process: MvProcessRef,
        world: *const MvCollisionWorld,
        shape: *const MvCollisionShape,
        dx: f64,
        dy: f64,
        out: *mut MvCollisionSweep,
    ) -> c_int;
    pub fn mv_collision_world_move(
        world: *const MvCollisionWorld,
        shape: MvCollisionShape,
        transform: *mut MvCollisionTransform,
        dx: f64,
        dy: f64,
        out: *mut MvCollisionSweep,
    ) -> c_int;
    pub fn mv_abi_version() -> u32;
    pub fn mv_version_string() -> *const c_char;
    pub fn mv_context_create(rate: f64, max_steps: u32) -> *mut MvContext;
    pub fn mv_context_destroy(ctx: *mut MvContext) -> ();
    pub fn mv_context_runtime_id(ctx: *mut MvContext, out: *mut MvRuntimeId) -> c_int;
    pub fn mv_context_advance(ctx: *mut MvContext, elapsed: f64, steps: *mut u32) -> c_int;
    pub fn mv_context_ticks(ctx: *const MvContext) -> u64;
    pub fn mv_context_dropped_seconds(ctx: *const MvContext) -> f64;
    pub fn mv_cadence_step(cadence: *mut MvCadence, fps: u32) -> c_int;
    pub fn mv_process_spawn(
        ctx: *mut MvContext,
        parent: u64,
        priority: i64,
        step: MvProcessStep,
        destroy: MvProcessDestroy,
        user: *mut c_void,
        out: *mut u64,
    ) -> c_int;
    pub fn mv_process_spawn_related(
        ctx: *mut MvContext,
        creator: u64,
        owner: u64,
        priority: i64,
        step: MvProcessStep,
        destroy: MvProcessDestroy,
        user: *mut c_void,
        out: *mut u64,
    ) -> c_int;
    pub fn mv_process_find(
        ctx: *mut MvContext,
        type_id: u64,
        out: *mut MvProcessInfo,
        capacity: usize,
        total: *mut usize,
    ) -> c_int;
    pub fn mv_process_children(
        ctx: *mut MvContext,
        root: u64,
        type_id: u64,
        out: *mut MvProcessInfo,
        capacity: usize,
        total: *mut usize,
    ) -> c_int;
    pub fn mv_process_descendants(
        ctx: *mut MvContext,
        root: u64,
        type_id: u64,
        out: *mut MvProcessInfo,
        capacity: usize,
        total: *mut usize,
    ) -> c_int;
    pub fn mv_process_creation_children(
        ctx: *mut MvContext,
        root: u64,
        type_id: u64,
        out: *mut MvProcessInfo,
        capacity: usize,
        total: *mut usize,
    ) -> c_int;
    pub fn mv_process_creation_descendants(
        ctx: *mut MvContext,
        root: u64,
        type_id: u64,
        out: *mut MvProcessInfo,
        capacity: usize,
        total: *mut usize,
    ) -> c_int;
    pub fn mv_process_lineage_snapshot(
        ctx: *mut MvContext,
        out: *mut MvLineageInfo,
        capacity: usize,
        total: *mut usize,
    ) -> c_int;
    pub fn mv_process_signal_creation_tree(ctx: *mut MvContext, root: u64, signal: c_int) -> c_int;
    pub fn mv_process_tick(ctx: *mut MvContext) -> c_int;
    pub fn mv_process_count(ctx: *const MvContext) -> usize;
    pub fn mv_process_signal(ctx: *mut MvContext, id: u64, signal: c_int, tree: c_int) -> c_int;
    pub fn mv_process_kill(
        ctx: *mut MvContext,
        reference: MvProcessRef,
        killed: *mut c_int,
    ) -> c_int;
    pub fn mv_process_ref_create(ctx: *mut MvContext, id: u64, out: *mut MvProcessRef) -> c_int;
    pub fn mv_process_ref_resolve(
        ctx: *mut MvContext,
        reference: MvProcessRef,
        out: *mut u64,
    ) -> c_int;
    pub fn mv_process_handle_create(
        ctx: *mut MvContext,
        reference: MvProcessRef,
        out: *mut *mut MvProcessHandle,
    ) -> c_int;
    pub fn mv_process_handle_snapshot(
        handle: *const MvProcessHandle,
        out: *mut MvProcessHandleInfo,
    ) -> c_int;
    pub fn mv_process_handle_destroy(handle: *mut MvProcessHandle) -> ();
    pub fn mv_process_info(
        ctx: *mut MvContext,
        reference: MvProcessRef,
        out: *mut MvProcessInfo,
    ) -> c_int;
    pub fn mv_process_set_priority(ctx: *mut MvContext, id: u64, priority: i64) -> c_int;
    pub fn mv_process_set_type(ctx: *mut MvContext, id: u64, type_id: u64) -> c_int;
    pub fn mv_resource_create(
        ctx: *mut MvContext,
        owner: *const MvProcessRef,
        data: *mut c_void,
        destroy: MvResourceDestroy,
        out: *mut *mut MvResource,
    ) -> c_int;
    pub fn mv_resource_transfer(resource: *mut MvResource, owner: *const MvProcessRef) -> c_int;
    pub fn mv_resource_retain(resource: *mut MvResource) -> c_int;
    pub fn mv_resource_release(resource: *mut MvResource) -> c_int;
    pub fn mv_resource_data(resource: *const MvResource) -> *mut c_void;
}

/// Check the linked library before calling any other engine operations.
/// Architecture and header compatibility are also checked at build time.
pub fn check_abi() -> Result<(), &'static str> {
    // SAFETY: version queries require no context and return static storage.
    unsafe {
        if mv_abi_version() != MV_ABI_VERSION {
            return Err("incompatible Malvado native ABI revision");
        }
        if CStr::from_ptr(mv_version_string()).to_bytes() != env!("CARGO_PKG_VERSION").as_bytes() {
            return Err("incompatible Malvado native release");
        }
    }
    Ok(())
}

#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct MvColor {
    pub r: u8,
    pub g: u8,
    pub b: u8,
    pub a: u8,
}
#[repr(C)]
#[derive(Clone, Copy, Debug, Default)]
pub struct MvGraphic {
    pub shape: MvCollisionShape,
    pub color: MvColor,
}
unsafe extern "C" {
    pub fn mv_process_graphic_set(
        ctx: *mut MvContext,
        process: MvProcessRef,
        graphic: MvGraphic,
    ) -> c_int;
    pub fn mv_process_graphic_get(
        ctx: *mut MvContext,
        process: MvProcessRef,
        graphic: *mut MvGraphic,
        present: *mut c_int,
    ) -> c_int;
    pub fn mv_process_draw_box(
        ctx: *mut MvContext,
        process: MvProcessRef,
        x: c_int,
        y: c_int,
        width: c_int,
        height: c_int,
        filled: c_int,
        color: MvColor,
    ) -> c_int;
    pub fn mv_process_draw_circle(
        ctx: *mut MvContext,
        process: MvProcessRef,
        x: c_int,
        y: c_int,
        radius: c_int,
        filled: c_int,
        color: MvColor,
    ) -> c_int;
    pub fn mv_process_draw_text(
        ctx: *mut MvContext,
        process: MvProcessRef,
        text: *const c_char,
        x: c_int,
        y: c_int,
        size: c_int,
        color: MvColor,
    ) -> c_int;
    pub fn mv_process_draw_count(
        ctx: *mut MvContext,
        process: MvProcessRef,
        out: *mut usize,
    ) -> c_int;
    pub fn mv_process_draw_clear(ctx: *mut MvContext, process: MvProcessRef) -> c_int;
    pub fn mv_process_graphic_clear(ctx: *mut MvContext, process: MvProcessRef) -> c_int;
}

// Match installed C layouts and constants for every consumer, not only tests.
use std::mem::{align_of, offset_of, size_of};
const _: () = {
    let native: &[usize] = &include!(concat!(env!("OUT_DIR"), "/layout.rs"));
    let rust = [
        size_of::<MvCadence>(),
        align_of::<MvCadence>(),
        offset_of!(MvCadence, phase),
        size_of::<MvRuntimeId>(),
        align_of::<MvRuntimeId>(),
        offset_of!(MvRuntimeId, high),
        offset_of!(MvRuntimeId, low),
        offset_of!(MvRuntimeId, sequence),
        size_of::<MvProcessRef>(),
        align_of::<MvProcessRef>(),
        offset_of!(MvProcessRef, runtime),
        offset_of!(MvProcessRef, id),
        size_of::<MvProcessInfo>(),
        align_of::<MvProcessInfo>(),
        offset_of!(MvProcessInfo, id),
        offset_of!(MvProcessInfo, parent),
        offset_of!(MvProcessInfo, type_id),
        offset_of!(MvProcessInfo, priority),
        offset_of!(MvProcessInfo, status),
        offset_of!(MvProcessInfo, messages),
        offset_of!(MvProcessInfo, creator_id),
        offset_of!(MvProcessInfo, attached),
        size_of::<MvProcessHandleInfo>(),
        align_of::<MvProcessHandleInfo>(),
        offset_of!(MvProcessHandleInfo, reference),
        offset_of!(MvProcessHandleInfo, alive),
        offset_of!(MvProcessHandleInfo, process),
        offset_of!(MvProcessHandleInfo, reason),
        MV_OK as usize,
        MV_INVALID_ARGUMENT as usize,
        MV_OVERFLOW as usize,
        MV_NOT_FOUND as usize,
        MV_OUT_OF_MEMORY as usize,
        MV_BUSY as usize,
        MV_PROCESS_ERROR as usize,
        MV_QUEUE_FULL as usize,
        MV_QUEUE_EMPTY as usize,
        MV_BUFFER_TOO_SMALL as usize,
        MV_SIGNAL_SLEEP as usize,
        MV_SIGNAL_FREEZE as usize,
        MV_SIGNAL_WAKEUP as usize,
        MV_SIGNAL_KILL as usize,
        MV_STEP_YIELD as usize,
        MV_STEP_DONE as usize,
        MV_STEP_ERROR as usize,
        MV_PROCESS_RUNNING as usize,
        MV_PROCESS_SLEEPING as usize,
        MV_PROCESS_FROZEN as usize,
        MV_END_KILLED as usize,
        MV_END_RETURNED as usize,
        MV_END_ERROR as usize,
        size_of::<MvLineageInfo>(),
        align_of::<MvLineageInfo>(),
        offset_of!(MvLineageInfo, id),
        offset_of!(MvLineageInfo, creator_id),
        offset_of!(MvLineageInfo, alive),
        size_of::<MvRect>(),
        align_of::<MvRect>(),
        offset_of!(MvRect, x),
        offset_of!(MvRect, y),
        offset_of!(MvRect, width),
        offset_of!(MvRect, height),
        size_of::<MvCollisionShape>(),
        align_of::<MvCollisionShape>(),
        offset_of!(MvCollisionShape, kind),
        offset_of!(MvCollisionShape, width),
        offset_of!(MvCollisionShape, height),
        offset_of!(MvCollisionShape, radius),
        offset_of!(MvCollisionShape, offset_x),
        offset_of!(MvCollisionShape, offset_y),
        size_of::<MvCollisionTransform>(),
        align_of::<MvCollisionTransform>(),
        offset_of!(MvCollisionTransform, x),
        offset_of!(MvCollisionTransform, y),
        offset_of!(MvCollisionTransform, angle),
        offset_of!(MvCollisionTransform, scale),
        size_of::<MvCollisionSweep>(),
        align_of::<MvCollisionSweep>(),
        offset_of!(MvCollisionSweep, hit),
        offset_of!(MvCollisionSweep, id),
        offset_of!(MvCollisionSweep, fraction),
        offset_of!(MvCollisionSweep, normal_x),
        offset_of!(MvCollisionSweep, normal_y),
        MV_COLLISION_CIRCLE as usize,
        MV_COLLISION_RECT as usize,
        size_of::<MvCameraViewport>(),
        align_of::<MvCameraViewport>(),
        offset_of!(MvCameraViewport, x),
        offset_of!(MvCameraViewport, y),
        offset_of!(MvCameraViewport, width),
        offset_of!(MvCameraViewport, height),
        size_of::<MvCamera>(),
        align_of::<MvCamera>(),
        offset_of!(MvCamera, x),
        offset_of!(MvCamera, y),
        offset_of!(MvCamera, viewport),
        size_of::<MvAnimationFrame>(),
        align_of::<MvAnimationFrame>(),
        offset_of!(MvAnimationFrame, graph_id),
        offset_of!(MvAnimationFrame, duration_us),
        size_of::<MvAnimationSample>(),
        align_of::<MvAnimationSample>(),
        offset_of!(MvAnimationSample, index),
        offset_of!(MvAnimationSample, graph_id),
        size_of::<MvAnimationState>(),
        align_of::<MvAnimationState>(),
        offset_of!(MvAnimationState, elapsed_us),
        offset_of!(MvAnimationState, remainder),
        offset_of!(MvAnimationState, tick_rate),
        size_of::<MvProcessDrawInfo>(),
        align_of::<MvProcessDrawInfo>(),
        offset_of!(MvProcessDrawInfo, id),
        offset_of!(MvProcessDrawInfo, z),
        size_of::<MvColor>(),
        align_of::<MvColor>(),
        offset_of!(MvColor, r),
        offset_of!(MvColor, g),
        offset_of!(MvColor, b),
        offset_of!(MvColor, a),
        size_of::<MvGraphic>(),
        align_of::<MvGraphic>(),
        offset_of!(MvGraphic, shape),
        offset_of!(MvGraphic, color),
        size_of::<MvCameraRef>(),
        align_of::<MvCameraRef>(),
        offset_of!(MvCameraRef, runtime),
        offset_of!(MvCameraRef, id),
    ];
    assert!(
        native.len() == rust.len(),
        "native ABI layout count mismatch"
    );
    let mut index = 0;
    while index < rust.len() {
        assert!(
            native[index] == rust[index],
            "native ABI layout/value mismatch"
        );
        index += 1;
    }
};

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn linked_core_checks_version_and_invalid_contexts() {
        assert_eq!(check_abi(), Ok(()));
        // SAFETY: the C contract permits invalid creation settings and NULL
        // destruction. This test owns and destroys the successful context.
        unsafe {
            assert!(mv_context_create(0.0, 5).is_null());
            let ctx = mv_context_create(60.0, 5);
            assert!(!ctx.is_null());
            let mut steps = 42;
            assert_eq!(
                mv_context_advance(ctx, -1.0, &mut steps),
                MV_INVALID_ARGUMENT
            );
            assert_eq!(steps, 42);
            assert_eq!(mv_process_tick(ctx), MV_OK);
            mv_context_destroy(ctx);
            mv_context_destroy(std::ptr::null_mut());
        }
    }

    #[test]
    fn callbacks_and_owned_handles_follow_native_cleanup() {
        #[derive(Default)]
        struct State {
            calls: u32,
            drops: u32,
        }
        unsafe extern "C" fn step(_: *mut MvContext, _: u64, user: *mut c_void) -> c_int {
            // SAFETY: this test keeps State live and exclusively callback-owned
            // during each native tick. The callback never unwinds across C.
            let state = unsafe { &mut *user.cast::<State>() };
            state.calls += 1;
            if state.calls == 2 {
                MV_STEP_DONE
            } else {
                MV_STEP_YIELD
            }
        }
        unsafe extern "C" fn destroy(user: *mut c_void) {
            // SAFETY: native cleanup happens before the test's State ends.
            unsafe {
                (*user.cast::<State>()).drops += 1;
            }
        }
        check_abi().unwrap();
        let mut state = State::default();
        // SAFETY: all native outputs use valid storage. Context and handle are
        // destroyed once; State remains live until after context destruction.
        unsafe {
            let ctx = mv_context_create(60.0, 5);
            assert!(!ctx.is_null());
            let mut id = 0;
            assert_eq!(
                mv_process_spawn(
                    ctx,
                    0,
                    0,
                    Some(step),
                    Some(destroy),
                    (&mut state as *mut State).cast(),
                    &mut id
                ),
                MV_OK
            );
            let mut reference = MvProcessRef::default();
            assert_eq!(mv_process_ref_create(ctx, id, &mut reference), MV_OK);
            let mut handle = std::ptr::null_mut();
            assert_eq!(mv_process_handle_create(ctx, reference, &mut handle), MV_OK);
            assert_eq!(state.calls, 0);
            assert_eq!(mv_process_tick(ctx), MV_OK);
            assert_eq!(state.calls, 1);
            assert_eq!(state.drops, 0);
            assert_eq!(mv_process_tick(ctx), MV_OK);
            assert_eq!(state.calls, 2);
            assert_eq!(state.drops, 1);
            mv_context_destroy(ctx);
            let mut info = MvProcessHandleInfo::default();
            assert_eq!(mv_process_handle_snapshot(handle, &mut info), MV_OK);
            assert_eq!(info.reference, reference);
            assert_eq!(info.alive, 0);
            assert_eq!(info.reason, MV_END_RETURNED);
            mv_process_handle_destroy(handle);
            assert_eq!(state.drops, 1);
        }
    }
}

#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub struct MvCameraViewport {
    pub x: c_int,
    pub y: c_int,
    pub width: c_int,
    pub height: c_int,
}
#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub struct MvCamera {
    pub x: f64,
    pub y: f64,
    pub viewport: MvCameraViewport,
}
#[repr(C)]
#[derive(Clone, Copy, Debug, Default)]
pub struct MvCameraRef {
    pub runtime: MvRuntimeId,
    pub id: u64,
}
unsafe extern "C" {
    pub fn mv_process_region_set(
        ctx: *mut MvContext,
        process: MvProcessRef,
        region: *const MvCameraViewport,
    ) -> c_int;
    pub fn mv_process_region_get(
        ctx: *mut MvContext,
        process: MvProcessRef,
        region: *mut MvCameraViewport,
        present: *mut c_int,
    ) -> c_int;
    pub fn mv_process_scroll_set(
        ctx: *mut MvContext,
        process: MvProcessRef,
        x: f32,
        y: f32,
    ) -> c_int;
    pub fn mv_process_scroll_get(
        ctx: *mut MvContext,
        process: MvProcessRef,
        x: *mut f32,
        y: *mut f32,
    ) -> c_int;
    pub fn mv_process_view_clip(
        ctx: *mut MvContext,
        process: MvProcessRef,
        host: *const MvCameraViewport,
        width: c_int,
        height: c_int,
        out: *mut MvCameraViewport,
    ) -> c_int;
    pub fn mv_process_view_offset(
        ctx: *mut MvContext,
        process: MvProcessRef,
        x: *mut f64,
        y: *mut f64,
    ) -> c_int;
    pub fn mv_camera_main(ctx: *mut MvContext, out: *mut MvCameraRef) -> c_int;
    pub fn mv_camera_create(ctx: *mut MvContext, value: MvCamera, out: *mut MvCameraRef) -> c_int;
    pub fn mv_camera_get(ctx: *mut MvContext, camera: MvCameraRef, out: *mut MvCamera) -> c_int;
    pub fn mv_camera_put(ctx: *mut MvContext, camera: MvCameraRef, value: MvCamera) -> c_int;
    pub fn mv_camera_close(ctx: *mut MvContext, camera: MvCameraRef) -> c_int;
    pub fn mv_process_camera_set(
        ctx: *mut MvContext,
        process: MvProcessRef,
        camera: *const MvCameraRef,
    ) -> c_int;
    pub fn mv_process_camera_get(
        ctx: *mut MvContext,
        process: MvProcessRef,
        out: *mut MvCameraRef,
    ) -> c_int;
    pub fn mv_process_camera_resolve(
        ctx: *mut MvContext,
        process: MvProcessRef,
        out: *mut MvCamera,
    ) -> c_int;
}

unsafe extern "C" {
    pub fn mv_camera_init(
        camera: *mut MvCamera,
        x: f64,
        y: f64,
        viewport: *const MvCameraViewport,
    ) -> c_int;
    pub fn mv_camera_set(camera: *mut MvCamera, x: f64, y: f64) -> c_int;
    pub fn mv_camera_move(camera: *mut MvCamera, dx: f64, dy: f64) -> c_int;
    pub fn mv_camera_project(
        camera: *const MvCamera,
        world_x: f64,
        world_y: f64,
        scroll_x: f64,
        scroll_y: f64,
        screen_x: *mut f64,
        screen_y: *mut f64,
    ) -> c_int;
    pub fn mv_camera_follow_step(
        camera: *mut MvCamera,
        ctx: *mut MvContext,
        target: *mut MvProcessRef,
        window_width: c_int,
        window_height: c_int,
    ) -> c_int;
    pub fn mv_camera_center(
        camera: *mut MvCamera,
        target_x: f64,
        target_y: f64,
        window_width: c_int,
        window_height: c_int,
    ) -> c_int;
    pub fn mv_camera_clip(
        camera: *const MvCamera,
        region: *const MvCameraViewport,
        screen_width: c_int,
        screen_height: c_int,
        out: *mut MvCameraViewport,
    ) -> c_int;
}

#[repr(C)]
pub struct MvText {
    _private: [u8; 0],
}
#[repr(C)]
pub struct MvAnimation {
    _private: [u8; 0],
}
#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub struct MvAnimationFrame {
    pub graph_id: c_int,
    pub duration_us: u64,
}
#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub struct MvAnimationSample {
    pub index: usize,
    pub graph_id: c_int,
}
#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub struct MvAnimationState {
    pub elapsed_us: u64,
    pub remainder: u32,
    pub tick_rate: u32,
}
unsafe extern "C" {
    pub fn mv_animation_create(
        frames: *const MvAnimationFrame,
        count: usize,
        r#loop: c_int,
    ) -> *mut MvAnimation;
    pub fn mv_animation_destroy(animation: *mut MvAnimation) -> ();
    pub fn mv_animation_sample(
        animation: *const MvAnimation,
        elapsed_us: u64,
        out: *mut MvAnimationSample,
    ) -> c_int;
    pub fn mv_animation_advance(
        animation: *const MvAnimation,
        state: *mut MvAnimationState,
        tick_rate: u32,
    ) -> c_int;
}

#[cfg(feature = "assets")]
pub mod assets;

#[cfg(feature = "graphics")]
pub mod graphics;

unsafe extern "C" {
    pub fn mv_text_create(
        value: *const c_char,
        x: c_int,
        y: c_int,
        alignment: c_int,
        out: *mut *mut MvText,
    ) -> c_int;
    pub fn mv_text_update(
        text: *mut MvText,
        value: *const c_char,
        x: c_int,
        y: c_int,
        alignment: c_int,
    ) -> c_int;
    pub fn mv_text_read(
        text: *const MvText,
        value: *mut *const c_char,
        x: *mut c_int,
        y: *mut c_int,
        alignment: *mut c_int,
    ) -> c_int;
    pub fn mv_text_destroy(text: *mut MvText);
    pub fn mv_text_aligned_x(x: c_int, width: c_int, alignment: c_int, out: *mut c_int) -> c_int;
    pub fn mv_graphic_rect(
        width: f64,
        height: f64,
        top_left: c_int,
        out: *mut MvCollisionShape,
    ) -> c_int;
    pub fn mv_graphic_circle(radius: f64, out: *mut MvCollisionShape) -> c_int;
}

#[repr(C)]
#[derive(Clone, Copy, Debug, Default)]
pub struct MvProcessDrawInfo {
    pub id: MvProcessId,
    pub z: f64,
}
unsafe extern "C" {
    pub fn mv_process_position(
        ctx: *mut MvContext,
        process: MvProcessRef,
        x: *mut f64,
        y: *mut f64,
    ) -> c_int;
    pub fn mv_process_set_position(
        ctx: *mut MvContext,
        process: MvProcessRef,
        x: f64,
        y: f64,
    ) -> c_int;
    pub fn mv_process_angle(ctx: *mut MvContext, process: MvProcessRef, out: *mut f64) -> c_int;
    pub fn mv_process_set_angle(ctx: *mut MvContext, process: MvProcessRef, value: f64) -> c_int;
    pub fn mv_process_scale(ctx: *mut MvContext, process: MvProcessRef, out: *mut f64) -> c_int;
    pub fn mv_process_set_scale(ctx: *mut MvContext, process: MvProcessRef, value: f64) -> c_int;
    pub fn mv_process_z(ctx: *mut MvContext, process: MvProcessRef, out: *mut f64) -> c_int;
    pub fn mv_process_set_z(ctx: *mut MvContext, process: MvProcessRef, z: f64) -> c_int;
    pub fn mv_process_draw_order(
        ctx: *mut MvContext,
        out: *mut MvProcessDrawInfo,
        capacity: usize,
        total: *mut usize,
    ) -> c_int;
}

unsafe extern "C" {
    pub fn mv_graphic_sprite(
        width: c_int,
        height: c_int,
        pivot_x: f64,
        pivot_y: f64,
        out: *mut MvCollisionShape,
    ) -> c_int;
}

unsafe extern "C" {
    pub fn mv_animation_retain(animation: *mut MvAnimation) -> c_int;
    pub fn mv_process_animation_clock_set(
        ctx: *mut MvContext,
        process: MvProcessRef,
        animation: *mut MvAnimation,
        tick_rate: u32,
    ) -> c_int;
    pub fn mv_process_animation_clock_get(
        ctx: *mut MvContext,
        process: MvProcessRef,
        state: *mut MvAnimationState,
        sample: *mut MvAnimationSample,
        present: *mut c_int,
    ) -> c_int;
    pub fn mv_process_animation_clock_clear(ctx: *mut MvContext, process: MvProcessRef) -> c_int;
}

unsafe extern "C" {
    pub fn mv_animation_count(animation: *const MvAnimation) -> usize;
}
