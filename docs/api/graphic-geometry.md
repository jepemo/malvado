# Simple graphic geometry

The core constructs rectangle and circle geometry without Lua, a window or a
backend. Lua `graphics.rect` and `graphics.circle` use these same constructors.
Dimensions must be finite and in `FLT_MIN..FLT_MAX/16`, including both limits.

Include `malvado/graphic.h` and link the core. `mv_graphic_rect(width, height,
top_left, out)` accepts 0 for a centered pivot and 1 for top-left. A top-left
rectangle has collider offsets `(width/2, height/2)`. `mv_graphic_circle(radius,
out)` is centered. Both return 1 on success and 0 for invalid input or NULL.
Failure leaves the output unchanged. Outputs are copied `MvCollisionShape`
values and own no resources.

Rust exposes `graphic::Geometry::rectangle(width, height, Pivot::Center)` and
`Geometry::circle(radius)`. Use `Pivot::TopLeft` for a corner origin. Constructors
return `InvalidArgument` on invalid geometry. `geometry.collider()` returns a
copied native collision shape for explicit collision queries or process collider
assignment. Copies are independent and need no cleanup. Default core-only Rust
consumers need no graphics feature to use them.

This service supplies geometry only. It does not assign a process graph, retain
an image, select a color, draw a scene, or synchronize a process collider.
Those compositions remain in A7.6.

`graphic_geometry` checks native boundaries, invalid pivots, NULL outputs and
atomic failure. Rust tests check pivot offsets through observable overlaps and
copied values. Standard/headless regressions preserve Lua constructor behavior.
The installed SDK probe checks both signatures and the existing collision-shape
layout. SDK consumers and native sanitizers exercise the same core constructors.
