# 2D cameras

Normal process graphics and immediate drawing use the main camera. It starts
at world position (0, 0). `self.camera = nil` selects it, `self.camera = false`
selects screen space, and a camera object selects that explicit camera. Camera
movement does not change process coordinates, colliders, scheduling, or z.

```lua
require("malvado").import_globals()
start(function()
  camera_set(100, 50)
  camera_move(10, 0)
end)
```

`camera_set(x, y)` and `camera_move(dx, dy)` use the main camera. Explicit calls
accept the camera first. Their module fields are aliases in `mv.camera`.
`camera_create(x, y, options?)` creates a managed camera. The optional `viewport`
is a `region(x, y, width, height)` descriptor; unknown options raise errors.

Use the object's `set`, `move` and `position` methods, or
`mv.camera.position(camera?)` to inspect coordinates (main by default).
Destroy an explicit camera with its `destroy` method. Destruction is idempotent;
operations and new assignments using that camera then raise errors. Existing
processes associated with it stop drawing until assigned a live camera, nil,
or false. Physics and process execution continue. The main camera cannot be
destroyed. Lua retains explicit cameras through process fields and defaults;
unreferenced cameras need no native resource cleanup.

For a fullscreen camera, projection is `world - camera - local_scroll`.
An explicit viewport adds its screen origin. The viewport clips the result;
a process region further restricts it by intersection in screen coordinates.
Projection occurs once for graphs and immediate drawing. Persistent text and
fades keep their existing screen-space behavior. Processes retain the existing
z/creation-order composition; screen space does not imply a separate HUD layer.
Use a larger z for an overlay above other processes.

Camera coordinates must be finite with magnitude at most `FLT_MAX / 64`.
Invalid moves leave the position unchanged. Local scroll retains its existing
bounds; an unrepresentable combined transform skips that process's drawing.
Camera functions work without a running process and in headless mode.

## Native C

```c
MvCamera camera;
if (mv_camera_init(&camera, 100, 50, NULL)) {
    double x, y;
    if (mv_camera_project(&camera, 120, 70, 0, 0, &x, &y)) {
        /* x and y are 20. Draw or inspect these coordinates. */
    }
}
```


Include `malvado/camera.h` and link `malvado_core`. `MvCamera` and
`MvCameraViewport` are caller-owned values with no allocation or cleanup.
`mv_camera_init` selects a fullscreen view with a NULL viewport. Explicit
viewport dimensions must be positive. `mv_camera_set` and `mv_camera_move`
modify position. `mv_camera_project` returns world minus camera minus local
scroll plus viewport origin. It does not clip or draw. Output pointers must
be distinct. Invalid pointers, nonfinite/out-of-range values, invalid viewports,
and unrepresentable results return 0 with outputs unchanged; success returns 1.
Do not modify a camera to an invalid value between calls.

| C symbols | Lua relationship | Evidence |
| --- | --- | --- |
| `mv_camera_init`, `mv_camera_set`, `mv_camera_move`, `mv_camera_project`, `mv_camera_center` | Main/explicit camera operations and projection | `tests/camera.c`, `tests/camera.lua`, installed core SDK consumer |
| `MvCamera`, `MvCameraViewport` | Native value state; Lua retains its own managed association | Same checks; no Lua or backend dependency |

## Follow a process

```lua
require("malvado").import_globals()
local Player = process(function(self)
  self.graph = circle(16)
  camera_follow(self)
  while true do
    self.x = self.x + 3
    frame()
  end
end)
start(function() Player() end)
```

`camera_follow(target)` tracks a live process's x/y origin with the main camera.
Use `camera:follow(target)` or `camera_follow(camera, target)` explicitly.
Following runs at the end of the existing simulation tick, after process updates
and removals, and before rendering. It centers the origin within the viewport,
or within the logical game width/height for a fullscreen camera. It does not
need a Lua controller process. It works in headless mode too.

A successful set/move cancels following; an invalid request leaves it intact.
A new follow target replaces the previous target. Calls take effect in scheduler
call order; the last successful request in a tick wins. Sleeping/frozen targets
remain valid and use their current position. A terminated target stops following
and retains the last camera position. Invalid/nonfinite/out-of-range target
positions retain the last valid position; tracking resumes when they become valid.
Destroying a camera releases its tracking reference. Tracking does not keep a
process alive or make it owned by the camera. Unreferenced explicit cameras can
be collected; the runtime's camera index is weak.

In C, call `mv_camera_center(camera, target_x, target_y, window_width, window_height)`
from the game's existing update flow. It uses viewport dimensions when present
and the supplied positive logical dimensions otherwise. It shares Lua's native
centering calculation. Invalid values return 0 without changing the camera.
C games own their positions; the native helper does not resolve Lua fields or
create a second scheduler. Smooth tracking and offset options are deferred.

## Camera, region and scroll together

A camera transforms its associated world content. A process scroll translates
only that process's drawing. A region is a fixed screen-space clip. For example,
with camera (100, 50), viewport origin (40, 60), local scroll (5, 5), and a
world point (120, 65), the projected point is (55, 70).

```lua
local area = region(40, 60, 300, 240)
local view = camera_create(100, 50, {viewport = area})
-- Inside a process:
self.camera = view
self.scroll = scroll(5, 5)
self.region = region(50, 70, 100, 80)
```

The effective clip is the intersection of viewport, process region and window.
It does not move with either scroll or camera position. Empty or wholly
out-of-window intersections skip that process's drawing. The renderer restores
both clip and translation before the next process. Regions and scroll do not
change physical process positions or collision geometry.

For a scrollable HUD inventory, set `self.camera = false`, keep a fixed
`self.region`, and replace `self.scroll` as the content moves. Its region remains
in screen coordinates. Other HUD/world processes keep their own transforms.
Draw graphs before that process's immediate commands, then continue in the
existing z/creation order across all cameras. Persistent text follows all
processes in screen space, and fades cover the final screen. These operations
do not create separate camera passes or a split-screen compositor.

C callers can use `mv_camera_clip(camera, region_or_null, width, height, &clip)`
for the same screen-space intersection. A visible result returns 1; an empty
intersection returns 0 and a zero rectangle. Invalid values return -1 without
changing the output. Intersection arithmetic handles viewport/region edges
outside the integer screen bounds without overflow. Clipping does not project
a point or change camera position. The renderer uses this native calculation.

| C symbols | Lua relationship | Evidence |
| --- | --- | --- |
| `mv_camera_clip` | Camera viewport, process region and screen intersection | `tests/camera.c`, installed SDK, `visual_camera_integration` |

The combined visual regression checks graph/immediate ordering, screen HUD z,
a clipped inventory, empty and off-screen views, independent cameras, destroyed
camera recovery, persistent text, and the final fade overlay. Native tests cover
projection, intersection, numeric boundaries and unchanged outputs on failure.


## Runnable examples

[Camera following](../../examples/camera-follow/) uses the main view and a fixed
HUD. [Clipped inventory](../../examples/camera-menu/) combines a screen-space
process, fixed region and local scroll while the world has a camera offset.
Both have automated behavior and pixel checks; the examples use both Lua entry
forms. See the [migration guide](../development/alpha6-api-migration.md).

## Native process follow update

`mv_camera_follow_step(&camera, ctx, &target, width, height)` centers a camera
from a process's native x/y. `target` is a context-bound `MvProcessRef`; an
all-zero reference means no target. Call once after the existing native tick
and process removals. The call does not tick, poll, allocate or retain a process.
Sleeping, frozen and not-yet-activated targets use their current native position.

When the target ends, the operation clears the reference and preserves the
last camera position. It returns `MV_OK` for an active update, a cleared dead
target or an inactive no-op. NULL inputs, malformed/foreign references, invalid
camera geometry, nonpositive window dimensions and out-of-range target geometry
return `MV_INVALID_ARGUMENT` without changing camera or follow state. Context
and pointer lifetimes remain the caller's responsibility. Viewport centering
uses the existing camera math. Copied target references are independent values.

Lua uses this operation at its existing end-of-tick update point for main and
explicit cameras. Lua keeps its target userdata reference to check local invalid
visual overrides. These overrides still preserve the last valid camera position.
`camera_set`, `camera_move`, destroy and target replacement keep their current
stop-following behavior. Main/explicit camera identity and process camera
selection remain Lua-owned; this service closes target tracking only.

`c_camera_follow` verifies foreign/NULL/malformed references, failure atomicity,
viewport/copy behavior, sleep/freeze, explicit kill, attached-child death, natural
return and exactly-once destruction while a follow reference still exists.
`alpha7_camera_follow_parity` compares C/Lua follow, frozen, viewport and ended
phases with one fixed golden trace. Existing headless and visual camera fixtures
check invalid local coordinates, replacement, shutdown and rendering behavior.

A7.6ad local macOS checks pass: C-only (59 tests), standard/headless
(175 tests each), and visual (57 tests). The expanded native destruction cases
and C/Lua parity also pass after the safe-point test correction. Linux CI and
native main/explicit camera ownership remain outside this local evidence.

## Context-owned camera identities and selection

`MvCameraRef` identifies a camera within one live context. Use
`mv_camera_main(ctx, &ref)` for the canonical main camera, initially at `(0, 0)`
with a full-window viewport. Main is ID 1 and cannot be closed.
`mv_camera_create(ctx, value, &ref)` copies a validated camera value into the
context. IDs are never reused. Get copies values; put replaces a complete value
atomically. Only create allocates. It returns `MV_OUT_OF_MEMORY` on allocation
failure, `MV_OVERFLOW` at ID exhaustion, and `MV_BUSY` during context shutdown.
Invalid values or NULL inputs return `MV_INVALID_ARGUMENT`. Failed operations
preserve outputs and stored state.

`mv_camera_close` frees an explicit camera. Repeated close of an issued identity
is successful; reading/writing a closed or unknown identity returns
`MV_NOT_FOUND`. Foreign references are invalid. Context shutdown releases every
remaining camera. Camera values own no graphics resources. All operations use
the owner thread. References are copied identities, not pointers or lifetime
retainers. Callers must keep the context live for native calls.

`mv_process_camera_set(ctx, process, camera)` selects a live same-context camera.
NULL selects main; an all-zero reference selects screen space. New processes
use main. Get returns the selected identity, including a closed explicit one.
Resolve returns a copied effective camera or `MV_NOT_FOUND` for a closed view.
A closed selected camera hides the process until reassignment. Camera changes
do not modify process coordinates, colliders, status or lifetime.

Lua retains `nil`/`false`/camera selection and its existing methods. All camera
positions and selections are now authoritative in C. `self.camera` reads native
selection and keeps existing userdata identity when available. Lua-created
camera userdata still closes its native camera on collection. A userdata view
of a camera created by C borrows it: collection drops only Lua metadata and
follow state. Explicit destroy still closes it. The context owns the underlying
camera until close or context shutdown. Process creation rechecks the selected
camera after preparation and before native spawn, so a stale native identity
cannot leave a process registered after a constructor error.

The native primitive process pass now applies the selected camera and viewport.
It intersects viewport and host clip, composes host scroll, and restores both
host scopes before return. Lua graph-before-command ordering remains in the existing runner.
Process-local region/scroll now use the shared native descriptors below. Camera follow timing
remains explicit; registration does not introduce a scheduler or automatic
updates outside the existing host tick flow.

`camera_registry` checks copied values, canonical main, close/repeated close,
foreign/dead/NULL errors, atomic invalid writes, screen/reset selection,
non-reused IDs and context cleanup with open cameras. The mixed C/Lua fixture
checks native read/write and selection, borrowed-view GC, native close and stale
spawn rejection without an orphan process. `visual_native_camera_registry`
checks native main/explicit/screen mutations, viewport clipping and closed-view
hiding on a frozen Lua process. Existing scope and camera visuals remain required.

A7.6al macOS evidence: C-only (63 tests), standard/headless (182 each) and
visual (60) pass. The final native primitive pixel test also checks that
camera/scroll cancellation cannot bypass the existing world-coordinate limit.
Installed probes verify all new signatures and CameraRef layout. One graphics
attempt failed in the existing keyboard visual fixture; the isolated fixture
and complete isolated graphics rerun passed without keyboard changes. Linux CI
and remaining asset/command services are not certified by this local evidence.

## Process region and local scroll

Each process stores an optional copied screen-space region and a local scroll.
New processes have no region and scroll `(0, 0)`. These values need no window,
resources or scheduler. They change drawing only: world position, colliders,
process state and lifetime stay unchanged.

Use `mv_process_region_set(ctx, ref, &region)` to copy a region; NULL clears it.
Dimensions must be positive. Origins can use the full signed integer range.
`mv_process_region_get` writes a presence flag and copies a present region. An
absent region leaves the rectangle output unchanged. Both output pointers are
required and their storage must not overlap.

`mv_process_scroll_set(ctx, ref, x, y)` copies finite floats with absolute values
at most `FLT_MAX / 16`. The getter requires two distinct non-NULL outputs.
Invalid inputs preserve stored values and outputs. These operations require a
live process in the same context on the owner thread. Dead processes return
`MV_NOT_FOUND`; NULL contexts and foreign references are invalid.

`mv_process_view_clip` intersects the selected camera viewport, process region,
optional host clip and logical screen. It writes a zero rectangle for an empty
intersection. Screen and clip dimensions must be positive, even when the
intersection is empty. `mv_process_view_offset` projects world origin through
the selected camera and local scroll. The drawing position is world position
plus this offset minus host scroll. Projection retains the camera numeric
limits. Both composition helpers return `MV_NOT_FOUND` for a closed selected
camera and preserve outputs on errors. Descriptor access remains available
until process death, including when its camera is closed.

Lua keeps `self.region = region_create(...)`, `self.scroll = scroll_create(...)`
and nil to reset. Reads use native values and retain an existing immutable
userdata when its value matches. Native zero scroll reads as nil unless a
matching explicit zero-scroll userdata is already stored. Native writes take
effect on the next draw, including frozen processes. Graphs and process drawing
commands use the same native descriptors. The native primitive pass composes
process and host scopes and restores host clip and scroll before return.

`process_view` checks defaults, copied values, composition, large integer
endpoints, numeric limits, invalid/foreign/dead calls and output preservation.
`native_lua_process_view` checks immediate mixed-language reads/writes and Lua
userdata identity. `visual_native_process_view` changes and clears native
region/scroll while a Lua actor is frozen, checks clipping/hiding/restoration,
and verifies unchanged world coordinates. Existing camera and scope visuals
also remain required.

A7.6an local macOS checks pass: C-only (64), standard/headless (184 each),
and visual (61). Installed Rust sys probes check all six native signatures.
This evidence does not certify Linux CI or the remaining A7.6 services.
