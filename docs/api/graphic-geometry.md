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

## Drawing

Enable Rust's `graphics` feature and call `frame.graphic(geometry, transform,
color)`. `transform` is a `collision::Transform` with position, clockwise angle
in degrees and positive scale. The geometry remains a copied value; drawing
retains no pointers. Clip and scroll scopes apply normally.

C uses `mv_window_draw_graphic(shape, transform, color)`. It accepts the
canonical shapes from `mv_graphic_rect/circle`, including centered or top-left
rectangles and centered circles. Arbitrary collider offsets are unsupported.
Transform values must be finite with magnitude at most `FLT_MAX/16`. Scale
must be positive; the scaled dimensions must remain in `FLT_MIN..FLT_MAX/16`.
Failure returns 0 without drawing or changing clip/scroll state. Rust reports
`InvalidArgument`. An active graphics frame is required; headless calls fail.

Lua primitive process graphs use the same native drawing operation. CMake's
graphics target now links the core constructors; Rust checks this installed
boundary and links the core archive after the backend for ordered linkers.

This service does not assign a process graph, retain an image, select a stored
color, collect process draws, or synchronize a process collider. Those
compositions remain in A7.6.

`graphic_geometry` checks native boundaries, invalid pivots, NULL outputs and
atomic failure. Rust tests check pivot offsets through observable overlaps and
copied values. Standard/headless regressions preserve Lua constructor behavior.
The installed SDK probe checks constructor/drawing signatures and the existing collision-shape
layout. SDK consumers and native sanitizers exercise the same core constructors.

The `graphic_visual` Rust fixture checks rendered scaled/rotated geometry,
center/corner pivots, circles, malformed native geometry, invalid transforms,
clip/scroll composition and restoration. The existing Lua process-graphics
visual fixture verifies the shared operation without changing Lua graph APIs.
The relocated graphics consumer also draws a geometry. These checks run in
Debug/Release and against the native ASan/UBSan graphics SDK.
