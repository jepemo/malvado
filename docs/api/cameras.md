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
