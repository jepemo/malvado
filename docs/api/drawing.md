# Draw 2D shapes

Use immediate drawing for shapes that a process calculates each time it runs.
Use a [reusable graphic](#process-graphics) when a shape should follow a
process's position, rotation, and scale.

```lua
require("malvado").import_globals()

local Overlay = process(function()
  while not key(KEY_ESCAPE) do
    draw_rect {x = 20, y = 20, width = 100, height = 60, color = BLUE}
    draw_circle {x = 180, y = 50, radius = 30, color = WHITE, filled = false}
    draw_line {x1 = 20, y1 = 90, x2 = 180, y2 = 90, color = YELLOW, thickness = 2}
    frame()
  end
end)
start(function() Overlay() end)
```

Call these functions inside a running process. Each takes one options table
and returns no values. The module has the same functions as
`mv.draw_rect(options)`, `mv.draw_circle(options)`, and `mv.draw_line(options)`.

| Function | Required fields | Optional fields |
| --- | --- | --- |
| `draw_rect` | `x` and `y` are the top-left corner. `width` and `height` are positive integers. | `filled` defaults to `true`. `color` defaults to the current drawing color. |
| `draw_circle` | `x` and `y` are the center. `radius` is a positive integer. | `filled` defaults to `true`. `color` defaults to the current drawing color. |
| `draw_line` | `x1`, `y1`, `x2`, and `y2` are the endpoints. | `thickness` defaults to 1 world pixel. `color` defaults to the current drawing color. |

Immediate shapes use integer coordinates in world pixels. The origin is at
the top left, and y increases downwards. A rectangle covers
`[x, x + width)` by `[y, y + height)`. At unit scale the last column is
`x + width - 1` and the last row is `y + height - 1`. Use `filled = false`
to draw just the perimeter. A rectangle one pixel wide or high has no interior.
Circles use the native one-pixel outline when `filled = false`.

Line thickness can be fractional. The stroke is centered on the segment
between the endpoint pixel centers. It has flat ends. Equal endpoints draw a
square centered on that pixel, with sides equal to the thickness.

Coordinates must fit a C `int`. Rectangle far edges and all four circle
bounds must also fit a C `int`. Thickness must be finite and between
`FLT_MIN` and `FLT_MAX / 16`, inclusive. Missing required fields, unknown
options, invalid types and out-of-range values raise a Lua error before the
command is added.

## Drawing colors and command lifetime

`set_color(color)` sets the default color for later rectangle, circle, and line
commands. The initial color is opaque white. This setting is shared by
processes. Pass `color` in the options to override it for one command. The
command copies its color, and the override leaves the shared default
unchanged. `draw_text` uses the same explicit color forms and defaults to opaque
white. See [text](text.md).

A process draws its graph first, followed by its commands in call order.
Commands use the process's `z`, camera, region, and scroll. They use the
coordinates you pass to them directly in that view. Each command stays visible
until the process runs again, even at a lower `fps`. Sleeping processes are
hidden. Frozen processes stay visible. Commands are released when the process
ends.

`mv.clear_screen(color)` sets the background for later frames. It validates
all supplied channels and uses only RGB. The background is always opaque.
For example, clearing to `TRANSPARENT` fills the window with opaque black.
Invalid input raises an error and leaves the background unchanged.

In C, call `mv_window_draw_box(x, y, width, height, filled, color)` or
`mv_window_draw_circle(x, y, radius, filled, color)` between `mv_window_begin`
and `mv_window_end`. They use the same geometry, with 1 for filled and 0 for
outlined. The functions return 0 for invalid geometry or no window. These
direct calls draw in call order.

## Lua colors

All Lua color inputs accept a built-in palette color or one of four table forms.
These four tables specify the same color.

```lua
local rgb = {255, 0, 0}
local rgba = {255, 0, 0, 255}
local named_rgb = {r = 255, g = 0, b = 0}
local named_rgba = {r = 255, g = 0, b = 0, a = 255}
```

Red, green, and blue are required. Alpha defaults to 255. Each channel must be
a Lua integer from 0 to 255. Numeric strings and floating-point values such
as `255.0` raise an error. Unknown fields, missing RGB channels, and tables
that mix named and positional keys also raise an error.

Alpha 0 is fully transparent and 255 is fully opaque for shapes, text, and
fade overlays. The screen background stays opaque.

Custom color tables are mutable. Set a channel before passing the table to
a drawing function.

```lua
local BACKGROUND = {r = 18, g = 22, b = 30}
local SOFT_BLUE = {90, 150, 215}
local fog = {180, 190, 210, 80}
fog[4] = 120
```

### Malvado palette

Use `mv.colors` with the namespaced API.

```lua
local mv = require("malvado")
mv.clear_screen(mv.colors.BLACK)
mv.set_color(mv.colors.RED)
```

Use the same constants as globals after importing them.

```lua
require("malvado").import_globals()
clear_screen(BLACK)
set_color(RED)
```

The constants are read-only native values. Read channels with either
`RED.r` or `RED[1]`. Alpha is available as `RED.a` or `RED[4]`. Assigning
to a channel raises an error. To make a mutable copy, use
`{RED.r, RED.g, RED.b, RED.a}`.

| Name | RGBA |
| --- | --- |
| `BLACK` | 0, 0, 0, 255 |
| `WHITE` | 255, 255, 255, 255 |
| `TRANSPARENT` | 0, 0, 0, 0 |
| `GRAY` | 128, 128, 128, 255 |
| `LIGHT_GRAY` | 211, 211, 211, 255 |
| `DARK_GRAY` | 64, 64, 64, 255 |
| `RED` | 255, 0, 0, 255 |
| `GREEN` | 0, 255, 0, 255 |
| `BLUE` | 0, 0, 255, 255 |
| `YELLOW` | 255, 255, 0, 255 |
| `CYAN` | 0, 255, 255, 255 |
| `MAGENTA` | 255, 0, 255, 255 |
| `ORANGE` | 255, 165, 0, 255 |
| `PURPLE` | 128, 0, 128, 255 |
| `PINK` | 255, 192, 203, 255 |
| `BROWN` | 165, 42, 42, 255 |

## Fade the screen

`mv.fade_off(500)` fades to opaque black over 500 ms. Call `mv.fade_on(500)`
to queue a return to transparent black. Both default to 250 ms. For another
color, call `mv.fade_to {color = mv.colors.BLUE, duration_ms = 500}`.

See [Fade transitions](fades.md) for queue behavior, timing, overlay order,
the C and Rust APIs, and migration from Lua tick durations.

## Process graphics

A process can draw a reusable circle or rectangle on every frame. Create a
descriptor once and put it in `graph`:

```lua
require("malvado").import_globals()
local graphics = require("malvado").graphics
local ball = graphics.circle {radius = 20, color = {80, 190, 255}}
local Ball = process(function(self)
  while not key("escape") do
    self.x = self.x + 1
    frame()
  end
end, {defaults = {graph = ball}})
start(function() Ball({x = 100, y = 100}) end)
```

`mv.graphics.rect {width = w, height = h, color = color, pivot = pivot}`
and `mv.graphics.circle {radius = r, color = color}` return immutable,
shareable descriptors. The dimensions are finite numbers in
`FLT_MIN..FLT_MAX / 16`. Fractional sizes are allowed. Both shapes default
to opaque white. A rectangle defaults to a `"center"` pivot and can use
`"top-left"`. A circle always uses its center. Missing required fields and
invalid dimensions, color, pivot, or option names raise an error. Lua keeps
the descriptor alive while any reference exists, including process defaults.

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
same constructors as `mv.graphics`. The flat module fields `mv.rect` and
`mv.circle` are also available. The geometric collision constructors stay
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
local viewport = mv.region {x = 20, y = 20, width = 160, height = 100}
local Actor = mv.process(function(self)
    while true do mv.frame() end
end, {defaults = {region = viewport, graph = mv.graphics.circle {radius = 20}}})
```

`region(options)` takes one table with four required fields. The coordinates
`x` and `y` must be integers in the C `int` range, which includes negative
origins. `width` and `height` must be positive integers up to `INT_MAX`.
Unknown fields raise an error. The returned descriptor is immutable and can
be shared by multiple processes, including in process defaults. Assign it to
`self.region` and set `self.region = nil` to clear it. Invalid assignments
raise an error and preserve the current region.

The region stays in logical screen coordinates when the process moves or
rotates, the camera moves, or local scroll changes. The effective clip is the
intersection of the region, window, and selected camera viewport. An empty
intersection hides that process's drawing.

The region clips the process graph and immediate draw commands, including
text and images. It does not clip persistent `write` text, other processes,
or the screen fade. It does not move objects, change draw order, or affect
collision geometry. Changes take effect at the next render. You can create
and assign descriptors in headless mode.

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

`scroll(x, y)` creates an immutable, shareable descriptor with an absolute
drawing offset. Assign it to `self.scroll` or include it in process defaults.
On the next render, the renderer subtracts `x` and `y` from the process graph
and immediate draw commands.
Coordinates are finite numbers with magnitude at most `FLT_MAX / 16`; the
descriptor stores them as C floats. Assign a new descriptor to replace the
offset, or set `self.scroll = nil` to reset it. Invalid values raise an error,
including during process creation. Descriptors can be created headlessly.

A process's `region` remains in window coordinates and clips the scrolled
result. Processes without the descriptor, persistent text, the fade, and
other processes are unaffected. Scroll does not change process positions,
collision geometry, scheduling, or `z` order. See [camera projection](cameras.md)
for how local scroll and the selected camera combine.


## Combine a process graphic with immediate shapes

Use the arrow keys to move the ball. The process renders the filled graphic
automatically and adds an outline and a line each time it runs.

```lua
require("malvado").import_globals()

local Player = process(function(self)
  self.graph = circle {radius = 20, color = GREEN}

  while not key(KEY_ESCAPE) do
    if key(KEY_LEFT) then self.x = self.x - 4 end
    if key(KEY_RIGHT) then self.x = self.x + 4 end
    if key(KEY_UP) then self.y = self.y - 4 end
    if key(KEY_DOWN) then self.y = self.y + 4 end

    draw_circle {x = self.x, y = self.y, radius = 24, color = WHITE, filled = false}
    draw_line {x1 = self.x, y1 = self.y, x2 = self.x + 35, y2 = self.y,
      color = YELLOW, thickness = 2}
    frame()
  end
end)

return {
  title = "Drawing Demo",
  width = 800,
  height = 600,
  init = function()
    clear_screen(BLACK)
    Player {x = 400, y = 300}
  end
}
```

## Migrate positional Lua drawing calls

- Replace `draw_box(x1, y1, x2, y2)` with `draw_rect {x = x1, y = y1, width = x2 - x1, height = y2 - y1}`.
- For the former positional `draw_rect`, also specify `filled = false`.
- Replace `draw_fcircle(x, y, radius)` with `draw_circle {x = x, y = y, radius = radius}`. For the former positional `draw_circle`, also specify `filled = false`.
- Move graphic dimensions into the options table. For example, `rect(32, 48, {color = RED})` becomes `rect {width = 32, height = 48, color = RED}` and `circle(20)` becomes `circle {radius = 20}`.

## Native process command collection

C hosts can append `mv_process_draw_box`, `mv_process_draw_circle`,
`mv_process_draw_line`, and `mv_process_draw_text` without a window. The line
takes two integer endpoints, a double thickness, and a color, with the same
bounds and rendering behavior as Lua. Text and colors are copied. Use
`mv_process_draw_image_region` for retained image regions and transforms.
The `_owned` variant also takes a binding lifetime owner on success. Native
images are released before the owner. Failure preserves the previous list and
caller ownership. Release commands before closing the window.

Commands keep append order and use world coordinates. They do not use the
selected graph's position, angle, scale, or collider. Rectangle far edges and
circle bounds must fit C `int`. Text keeps the immediate built-in renderer behavior:
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
