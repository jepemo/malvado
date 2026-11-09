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
