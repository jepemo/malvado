# Rust camera and animation values

These services use the installed C core. They need no graphics backend. They do
not load graphs or update a process automatically. Context-owned camera
identities and process selection are available below.

## Cameras

```rust
use malvado::camera::Camera;
let mut camera = Camera::new(0.0, 0.0, None)?;
camera.center((200.0, 150.0), (800, 600))?;
let screen = camera.project((210.0, 160.0), (0.0, 0.0))?;
assert_eq!(screen, (410.0, 310.0));
# Ok::<(), malvado::Error>(())
```

`Camera` owns a copied native value. Copying it creates an independent camera.
`None` in the constructor uses the full window. An explicit `Viewport` must have
positive dimensions. Coordinates must be finite and within `FLT_MAX / 64`.
Projection applies world minus camera minus local scroll plus viewport origin.
`clip` intersects the viewport, optional region, and screen without projection;
it returns `None` for an empty intersection. Screen dimensions must be positive.
Invalid inputs return `Error::InvalidArgument` and preserve camera state.
No allocation or cleanup is required. The game controls when to center or move
its camera; these values do not add an update loop.

## Registered cameras and process selection

`engine.main_camera()?` returns the canonical context-owned main camera.
`engine.create_camera(value)?` registers a copied Camera value and returns a
`CameraHandle`. Clones share identity; `snapshot()` returns an independent
Camera copy. `replace`, `set_position` and `move_by` update native values.

```rust
use malvado::{Engine, Error, camera::{Camera, Selection}};
let engine = Engine::new()?;
let actor = engine.spawn(Default::default(), |p| async move {loop {p.frame().await;}})?;
let view = engine.create_camera(Camera::new(10.0, 20.0, None)?)?;
actor.set_camera(Selection::Explicit(view.clone()))?;
view.move_by(1.0, 2.0)?;
assert_eq!(actor.resolve_camera()?.position(), (11.0, 22.0));
view.close()?;
assert!(matches!(actor.resolve_camera(), Err(Error::NotFound)));
actor.set_camera(Selection::Screen)?;
assert_eq!(actor.resolve_camera()?.position(), (0.0, 0.0));
# Ok::<(), Error>(())
```

Use `Selection::Main`, `Screen` or `Explicit(handle)` with `set_camera`.
`camera_selection` copies identity, including a closed explicit selection.
`resolve_camera` copies the effective value; a closed selected view returns
`NotFound` and hides that process in the native primitive drawing pass.
Foreign context selection returns `ForeignContext` without changing the current
selection. Main cannot be closed (`InvalidArgument`). Explicit close is
idempotent. After engine shutdown camera operations return `Closed`.

The context owns registrations until close or shutdown. Dropping a handle does
not close the view, so selected processes keep their camera when a temporary
handle leaves scope. Use close to release an unneeded explicit view. Handles
stay on the owner thread, while copied Camera snapshots remain independent.
No window is needed for these services. Registration/selection never ticks,
starts a scheduler, follows a target or changes process world coordinates.

`frame.process_graphics(&engine)?` now applies native selection and viewport.
It composes and restores host clip/scroll; the default full-window case remains
batched. Per-process native region/command storage remains A7.6 work. Immediate
frame drawing is still explicit; use a snapshot's project/clip operations for
custom drawing. Lua-owned and borrowed camera userdata use the same core; see
[the native ownership contract](cameras.md#context-owned-camera-identities-and-selection).

`Follow::update_handle(&camera, window_size)?` applies an existing Follow value
to a registered camera. Both identities must belong to the same context. Call
it after the native tick, as for copied cameras. Errors preserve native camera
values; a closed camera returns `NotFound` even for an inactive follower. Manual
camera writes do not clear the separate Follow object: call `clear()` to stop.

`camera_registry.rs` verifies identity, aliases versus snapshots, explicit close,
main protection, selection/reset, foreign/dead/closed errors, follow updates and
context cleanup with open cameras. Owner-thread and runnable doctests check the
public usage. Native and Rust pixel fixtures verify main/explicit/screen views,
closed-view hiding, viewport boundaries and host scope composition. Installed
core/graphics consumers exercise registered selection through relocated SDKs.

## Follow a process

`Follow::new(&actor)?` selects a live context-bound handle without moving a
camera. Call `follow.update(&mut camera, (width, height))?` after the existing
engine tick. It reads native x/y, uses viewport centering and returns `true`
while the target exists. Sleep and freeze do not stop explicit follow updates.
When the target ends, update clears the target, returns `false` and preserves
the last camera position. It does not tick or poll any future.

```rust
use malvado::{Engine, camera::{Camera, Follow}};
let mut engine = Engine::new()?;
let actor = engine.spawn(Default::default(), |p| async move {
    loop { p.frame().await; }
})?;
actor.set_position(100.0, 80.0)?;
let mut camera = Camera::new(0.0, 0.0, None)?;
let mut follow = Follow::new(&actor)?;
engine.tick()?;
assert!(follow.update(&mut camera, (100, 80))?);
assert_eq!(camera.position(), (50.0, 40.0));
# Ok::<(), malvado::Error>(())
```

`Follow` owns a handle, not the target process or its resources. Creating it
from a dead/closed handle returns `NotFound`/`Closed`. Invalid dimensions or
out-of-range geometry return `InvalidArgument`; errors preserve camera and
follow state. After engine shutdown, an active update returns `Closed`. Call
`clear()` to stop and release the handle. A cleared follower returns `false`
on update with positive dimensions, even after engine shutdown.
`is_following()` reports whether a target remains selected, before the next
update checks its liveness.

Camera values remain independently copied. Follow state is separate: moving or
centering a Rust camera does not clear it. Call `clear()` when switching to
manual control, or replace the follower with `Follow::new(&other)?` to retarget.
Followers stay on the owner thread. Independent followers can select the same
target without retaining process state. Context-owned camera identity and process selection are available above;
following still uses the explicit host update flow.

`camera.rs` tests follow the C/Lua golden trace and check viewport behavior,
sleep/freeze, invalid geometry/dimensions, independent follow state, explicit
clear, closed engines and natural process cleanup. A compile-fail doctest
checks thread ownership; a runnable doctest checks the tick/update sequence.
The installed probe checks `mv_camera_follow_step`; the relocated external
consumer exercises real follow centering without source-tree SDK paths.

## Animations

```rust
use malvado::animation::{Animation, Frame};
let animation = Animation::new(&[
    Frame { graph_id: 1, duration_us: 100_000 },
    Frame { graph_id: 2, duration_us: 100_000 },
], true)?;
let mut playback = animation.playback();
// Call once in the game's existing simulation tick.
let sample = playback.advance(60)?;
assert_eq!(sample.graph_id, 1);
# Ok::<(), malvado::Error>(())
```

The definition copies 1..8192 frames into native storage and frees it on drop.
Graph IDs must be positive; durations are 1..3600000000 microseconds. Invalid
frames return `InvalidArgument`; native allocation failure returns `OutOfMemory`.
Graph IDs do not load or retain graphical assets. Definitions stay on their
creating thread and can be held by the existing `Resource<Animation>` manager.

`sample(elapsed_us)` samples without changing state. Each `playback()` starts at
zero and borrows its definition, so it cannot outlive the native allocation.
`advance(rate)` calls the native fixed-tick accumulator; rates are 1..1000000.
Fractional microseconds carry across ticks without drift. Call `reset()` before
changing rate. Invalid rates leave state unchanged. Looping playback wraps;
non-looping playback holds its last frame. This service does not assign a graph
or advance automatically when a process runs, sleeps, or freezes.

The Rust tests check boundaries, native frame copying, invalid definitions,
independent playback, exact tick accumulation, rate errors, loop/end behavior,
and retention after context shutdown. Compile-fail examples verify playback
lifetime and owner-thread destruction. Installed-header probes check every
camera/animation signature and all public record layouts and offsets.

Local verification: `mise run test-rust` passes in Debug and Release, including
51 workspace tests, 11 doctests, Clippy, format, bounded existing examples,
SDK rejection fixtures, and an offline external consumer that uses both modules
from a relocated installed SDK. `mise run test-rust-sanitize` passes with the
new native operations; Rust itself is not sanitizer-instrumented.

A7.6ad/ae host evidence on macOS: C-only (59 tests), standard/headless
(175 each), visual (57), Rust Debug/Release, relocated external consumer,
installed signature checks, Clippy/format and native ASan/UBSan integration
pass. The runnable Follow example and owner-thread compile failure also pass
as doctests in both profiles. Linux CI and native main/explicit camera ownership
remain open; these checks do not certify automatic Rust process camera selection.

A7.6am macOS evidence: Rust core and graphics Debug/Release suites pass with
registered-camera tests and relocated installed consumers. The registered-camera
usage and owner-thread doctests pass in both profiles. Clippy/format and native
graphical ASan/UBSan pass, including open-camera context cleanup and viewport
pixels; Rust itself is not instrumented. Native scope/coordinate-boundary pixel
checks pass after the default full-window batching adjustment. The isolated
full graphics rerun passed after an earlier keyboard fixture failure. Linux CI
and the remaining asset/command/region services remain open.
