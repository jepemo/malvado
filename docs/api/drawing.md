# Draw rectangles and circles

A process can draw filled or outlined shapes before it calls `mv.frame()`. Coordinates and radii use integer window pixels. The origin is at the top-left corner.

```lua
local mv = require("malvado")
local Shape = mv.process(function()
  while not mv.key("escape") do
    mv.set_color(60, 160, 255)
    mv.draw_box(20, 20, 120, 80)
    mv.set_color(255, 255, 255, 180)
    mv.draw_circle(180, 50, 30)
    mv.frame()
  end
end)
mv.start(function() Shape() end)
```

`set_color(r, g, b, a)` sets the color for later primitive commands. Alpha defaults to 255. Each channel is an integer from 0 to 255. The color is shared by processes, so set it before drawing when a particular color is needed. A queued command keeps the color it had when it was created. The initial color is opaque white.

- `mv.draw_box(x1, y1, x2, y2)` draws a filled rectangle. `mv.draw_rect` draws its outline. The second corner must be below and to the right of the first.
- `mv.draw_fcircle(x, y, radius)` draws a filled circle. `mv.draw_circle` draws its outline. The radius must be positive.

Invalid geometry raises an error. Draw commands are valid only inside a running process. They follow the process's `z` order and persist until its next activation. The legacy compatibility import also provides these functions as globals.

In C, call `mv_window_draw_box(x, y, width, height, filled, color)` or `mv_window_draw_circle(x, y, radius, filled, color)` between `mv_window_begin` and `mv_window_end`. Pass 1 for filled or 0 for outlined. The functions return 0 for invalid geometry or no window. C drawing follows call order.

`mv.clear_screen(r, g, b)` sets the background for later frames. Each channel
must be an integer from 0 to 255. Invalid channels raise an error and leave the
background unchanged.

## Fade the screen

`mv.fade_off(ticks)` fades to opaque black, and `mv.fade_on(ticks)` fades back to transparent. The duration defaults to 16 simulation ticks. Calls are queued, so `mv.fade_off(); mv.fade_on()` runs the two transitions in order. The legacy import also supplies these two names. Its argument is a duration in ticks; it does not reproduce the historical speed argument.

For another color, call `mv.fade_to(r, g, b, a, ticks)`. Channels are integers from 0 to 255; duration is an integer from 0 to 1,000,000 ticks. A duration of zero changes the color immediately when the request reaches the front of the queue. Up to 16 requests can wait behind the current transition; adding another raises an error. `mv.fade_active()` is true while a transition or queued request remains. `mv.fade_color()` returns the current red, green, blue, and alpha values. The overlay is drawn after processes and persistent text. It advances once per simulation tick and stops when the runner stops.

In C, initialize `MvFade fade = {0}` and call `mv_fade_start(&fade, target, ticks)`. Call `mv_fade_step(&fade)` on each simulation tick and use `fade.current` as the color of a window-sized filled box. `mv_fade_active(&fade)` reports whether the transition is in progress. A start with invalid input returns 0 without changing the state. C callers manage any transition queue themselves.

## Process graphics

A process can draw a reusable circle or rectangle on every frame. Create a
descriptor once and put it in `graph`:

```lua
require("malvado").import_globals()
local graphics = require("malvado").graphics
local ball = graphics.circle(20, {color = {80, 190, 255}})
local Ball = process(function(self)
  while not key("escape") do
    self.x = self.x + 1
    frame()
  end
end, {defaults = {graph = ball}})
start(function() Ball({x = 100, y = 100}) end)
```

`graphics.circle(radius, options)` and `graphics.rect(width, height, options)`
return immutable descriptors. Dimensions must be finite, positive, and in
`FLT_MIN..FLT_MAX / 16` so the graphics backend can represent them. Options may
contain `color = {r, g, b}` or `{r, g, b, a}`; each channel is an integer in
0..255. The default is opaque white. `pivot` defaults to `"center"` for both
shapes. Rectangles also accept `"top-left"`. Circles only accept `"center"`.
Invalid dimensions, colors, pivots, or unknown options raise an error. Lua
retains the descriptor while it is referenced, including by process defaults.
Descriptors can be shared between processes without sharing mutable fields.

A graph uses the process `x`, `y`, clockwise `angle` in degrees, and uniform
`size` (default 1). `z` keeps its existing draw-order meaning. The graph is drawn
before that process's immediate draw commands. Sleeping processes are hidden;
frozen processes remain visible. `graph = nil` removes the graphic.
Invalid or out-of-range transform values, non-positive size, or scaled dimensions
outside `FLT_MIN..FLT_MAX / 16` skip the graph for that frame. Process field
assignments otherwise keep their existing rules. Values other than supported graphics or images are ignored by automatic drawing.
See [images](images.md#use-images-and-atlas-sprites-as-process-graphics) for
image and atlas sprite graphics.

Without an explicit collider, a supported graphic also supplies its collision
shape, using the same pivot and transform. See [collision](collision.md).
`import_globals()` installs `rect`, `circle`, `region`, and `scroll` as the
same constructors as `mv.graphics`. The geometric collision constructors stay
in `mv.collision`; calling the imported `collision(target)` queries processes.

In C, the raylib platform provides `mv_window_draw_transformed_rect` and
`mv_window_draw_filled_circle` between `mv_window_begin` and `mv_window_end`.
The rectangle takes its pivot in local pixels and rotates clockwise. These
functions draw immediately and do not create colliders. They return 0 for
invalid input or no window, including in the headless backend.

## Window clipping in C

Between `mv_window_begin` and `mv_window_end`, call
`mv_window_clip_begin(x, y, width, height)` to restrict subsequent drawing to
one rectangle in logical window pixels. The rectangle must be nonempty and
fully inside the window. Clips cannot be nested. Invalid arguments or state
return 0 without changing the current clip. `mv_window_clip_end()` restores
unrestricted drawing and returns 1, or 0 if no clip is active. Ending a frame
also clears an active clip. These calls return 0 without a graphics window.
This low-level primitive does not implement scrolling.

## Clip a Lua process to a region

Create a reusable region in logical window pixels and assign it to a process:

```lua
local mv = require("malvado")
local viewport = mv.graphics.region(20, 20, 160, 100)
local Actor = mv.process(function(self)
    while true do mv.frame() end
end, {defaults = {region = viewport, graph = mv.graphics.circle(20)}})
```

`graphics.region(x, y, width, height)` accepts integer coordinates in the C
`int` range and positive integer dimensions up to `INT_MAX`. It creates an
immutable descriptor without opening a window. Descriptors can be shared in
process defaults. `self.region` accepts a descriptor or `nil`; `nil` removes the
clip. Invalid descriptors and invalid assignments raise an error, including
at process creation. The region stays in window coordinates when the process
moves or rotates. Parts outside the window are clipped to its bounds, and a
fully outside region hides that process's drawing.

The region clips the process graph and immediate draw commands, including
text and images. It does not clip persistent `write` text, other processes,
or the screen fade. It does not move objects, change draw order, or affect
collision geometry. Changes take effect at the next render; headless
simulation does not require a window. Regions are independent of the scroll
feature, which remains pending.

## Window scrolling in C

Between `mv_window_begin` and `mv_window_end`, call
`mv_window_scroll_begin(x, y)` to translate world drawing by `(-x, -y)`.
Coordinates must be finite and have magnitude at most `FLT_MAX / 16`.
`mv_window_scroll_end()` restores window coordinates. A frame end also
restores the state. Calls return 1 on success or 0 for invalid arguments or
state. Scrolling cannot be nested. If a clip is needed, begin it before the
scroll and end it after the scroll. Invalid calls leave both states unchanged.
These functions return 0 when there is no graphics window. They do not change
process positions or collisions.

## Scroll a Lua process

`graphics.scroll(x, y)` creates an immutable, shareable scroll descriptor.
Assign it to `self.scroll` or include it in process defaults. It translates the
process graph and immediate draw commands by `(-x, -y)` on the next render.
Coordinates are finite numbers with magnitude at most `FLT_MAX / 16`; the
descriptor stores them as C floats. Assign a new descriptor to move the
view, or `nil` to restore window coordinates. Invalid values raise an error,
including during process creation. Descriptors can be created headlessly.

A process's `region` remains in window coordinates and clips the scrolled
result. Processes without the descriptor, persistent text, the fade, and
other processes are unaffected. Scroll does not change process positions,
collision geometry, scheduling, or `z` order. This initial capability is a
uniform translation; it does not implement wrapping, parallax, tilemaps, zoom,
or camera rotation.


## Minimal graphic constructors

```lua
require("malvado").import_globals()
local Player = process(function(self)
  self.graph = circle(20, {color = {80, 190, 255}})
  self.region = region(40, 60, 300, 240)
  self.scroll = scroll(10, 0)
  while true do frame() end
end)
start(function() Player({x = 100, y = 100}) end)
```

`rect(width, height, options?)`, `circle(radius, options?)`,
`region(x, y, width, height)`, and `scroll(x, y)` are available after
`import_globals()`. The flat module fields (`mv.rect`, `mv.circle`, `mv.region`,
`mv.scroll`) and existing `mv.graphics` fields contain the same functions.
They return the same immutable descriptors and use the validation and ownership
rules above. Shapes default to a center pivot and opaque white; rectangles can
select `pivot = "top-left"`. RGB and RGBA color tables are supported. Unknown
shape options raise an error. No `shape` namespace is required.

This increment preserves immediate drawing: `draw_rect` and `draw_box` take
two corners, and filled/outlined circles keep their existing functions.
`set_color` affects immediate primitives; it does not change a new descriptor's
default color. A proposal to change those signatures needs a separate decision.
An outlined drawing does not change collision geometry into a hollow shape.


## Native process command collection

C hosts can append `mv_process_draw_box`, `mv_process_draw_circle`, and
`mv_process_draw_text` without a window. Text and colors are copied. Use
`mv_process_draw_image_region` for retained image regions and transforms.
The `_owned` variant also takes a binding lifetime owner on success. Native
images are released before the owner. Failure preserves the previous list and
caller ownership. Release commands before closing the window.

Commands keep append order and use world coordinates. They do not use the
selected graph's position, angle, scale, or collider. Box corners and circle
bounds must fit C `int`. Text keeps the immediate built-in renderer behavior:
raylib uses a minimum size of 10 pixels, including for nonpositive values.
`mv_process_draw_count` reports the list length. `mv_process_draw_clear` releases
the list. Clear is idempotent. Kill, return, and context cleanup also release it.
Ticks and signals do not clear commands. A language host must clear immediately
before each actual process activation, after its cadence check.

`mv_window_draw_process_graphics` draws each selected graph, then that process's
commands, in native z/creation order. Sleeping processes are hidden; frozen
processes remain visible. Camera, region, local scroll, and host scopes apply.
An invalid graph transform does not hide independent commands. The pass restores
host scopes and flushes borrowed texture uses before return.
`mv_window_draw_process_commands` draws only one list in the current scopes;
the caller supplies visibility and projection. Both require an active frame.
Collection does not add a scheduler.


## Shared Lua command storage

Lua primitive, immediate text, whole-image and region/sprite commands use the
native list in call order. Lua clears the previous list only before resuming the
coroutine, after its cadence check. Sleeping/frozen and low-cadence processes
keep the existing visibility and persistence rules. The graph draws first.
Image commands also keep Lua image userdata reachable until the next activation
or cleanup. These Lua roots contain no duplicated command fields. Text is copied
by C. Lua inspection reads native command counts, including commands added by C.
Native clear is visible immediately on frozen Lua processes.

## Shared text alignment

`mv_text_aligned_x(x, width, alignment, &out)` in `malvado/graphic.h`
computes the horizontal origin from a measured pixel width. Use 0 for left,
1 for center, or 2 for right. Center subtracts `width / 2` with integer
rounding. The result saturates at `INT_MIN`. A negative width, unknown
alignment, or NULL output returns 0 and leaves the output unchanged.
The operation requires no window, allocation, or process.

Lua persistent text uses this operation after built-in or custom font
measurement. Its `left`, `center`, and `right` behavior is unchanged.

Rust exposes the same core operation through
`malvado::graphic::TextAlignment::{Left, Center, Right}`:

```rust
use malvado::graphic::TextAlignment;
assert_eq!(TextAlignment::Center.aligned_x(100, 25)?, 88);
# Ok::<(), malvado::Error>(())
```

Negative widths return `Error::InvalidArgument`. Width measurement remains
explicit. This value helper does not create or retain persistent text objects.
