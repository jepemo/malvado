# Drawing and coordinates

Draw an outlined rectangle, then use a horizontal line to connect its
lower-left edge to the center of a circle. This gives you three ways to
place shapes in a window without loading any artwork.

## Run the example

Install the [desktop SDK](../../start/installation/) and download
`shapes.lua`. Open a terminal in the download directory.

```sh
malvado run shapes.lua
```

Look for the blue outline, white line and orange circle. Press Escape or
close the window to exit.

<!-- example -->

## Put a shape where you want it

At the default camera position, (0, 0) is the top-left of the logical window.
Positive x goes right. Positive y goes down. The rectangle starts at
(24, 24) and is 72 pixels wide by 48 pixels high. Its last row is y = 71,
so the line starts just below it at y = 72.

The circle has a radius of 16 pixels. Its process coordinates name its center.
The rectangle's immediate draw call uses its top-left corner. Check the
anchor and the size units before moving a shape. The color table gives red,
green and blue channels from 0 to 255. A fourth channel gives opacity, where
0 is transparent and 255 is opaque. Named palette colors work too.

## Decide what to redraw

The circle is a process graphic. Assign it to `self.graph` once and it
follows the process coordinates until you change or clear it. The line and
rectangle are immediate draw commands. Malvado keeps them until the process
runs again, so submit them on each activation that should show them. The
coordinates in a draw call are already in the active view's coordinate space.

Move the `mv.draw_rect` call above the loop. It will show on the first turn
and disappear on the next. Next change `self.x` to 210. The circle moves
while the line endpoint stays at 180.

See the [Lua drawing reference](../../reference/lua/drawing/) and the
[drawing guide for C and Lua](../../concepts/drawing/). Experimental Rust
has [drawing and image APIs](../../concepts/rust-drawing/). Continue with
[input and movement](input-and-movement.md).
