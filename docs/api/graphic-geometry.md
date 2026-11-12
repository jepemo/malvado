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

The constructors remain copied geometry. Primitive process assignment and
drawing are available below. Image/animation graph retention, process command
collection and native automatic collider synchronization remain in A7.6.

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

## Primitive process graphs

`MvGraphic` copies canonical rectangle/circle geometry and `MvColor`. Color is
now declared in `graphic.h`; it remains available through `platform.h` with its
unchanged four-byte layout. The core needs no window or graphics dependency.

Use `mv_process_graphic_set(ctx, reference, graphic)` to assign a primitive,
`mv_process_graphic_get(ctx, reference, &graphic, &present)` to copy it, and
`mv_process_graphic_clear(ctx, reference)` to remove it. New processes have no
primitive. An absent get writes zero to `present` and leaves `graphic` intact.
Both output pointers must be non-NULL. Clear is idempotent. The service validates
canonical geometry and normalizes unused shape fields; arbitrary collider
offsets are not accepted. Failure leaves state and outputs unchanged. Invalid
or foreign inputs return `MV_INVALID_ARGUMENT`; ended processes return
`MV_NOT_FOUND`. All calls require a live owner-thread context and allocate
nothing. Assignment transfers no resource or pointer ownership.

Lua primitive `self.graph` defaults, spawn options, setters, getters, render
reads and derived collider queries share this native descriptor. Readback keeps
the original immutable userdata while its geometry/color still match. A native
replacement appears on the next read, draw or collider synchronization, including
for frozen processes. Native clear removes a cached primitive. Lua assigning an
image, sprite, animation or invalid graph value clears the primitive slot and
keeps the existing binding behavior. Native primitive clear does not remove a
Lua image/animation graph: their native storage/retention is still pending.
Fixed C/Rust registrations remain independent. Bound primitive colliders now
follow native graph/visual changes; see the [selection contract](collision.md#bound-process-colliders).

`process_graphic` checks copied state, defaults, clear, canonical validation,
NULL/foreign/dead references, atomic errors and frozen persistence. The mixed
C/Lua host test checks bidirectional assignment, userdata readback, native clear
and derived collider refresh. `visual_native_process_graphic` checks native
replacement/color/extent and clearing on a frozen Lua process.

## Draw stored primitive graphs

`mv_window_draw_process_graphics(ctx)` draws the native primitive graphs in
ascending z/creation order during an active frame. Sleeping processes hide;
frozen and not-yet-activated processes are included. Transform and scale limits
match `mv_window_draw_graphic`; unsupported or nonpositive-scale transforms are
skipped. Registered process camera/viewport selection and copied process region/scroll
apply. Current host clip/scroll scopes compose with those descriptors and are
restored before return. See [process views](cameras.md#process-region-and-local-scroll).
There is no implicit process activation, tick, immediate-command collection,
text, fade or asset draw.
Call this explicit pass at the chosen point in the host's render flow.

The operation allocates one ordered process snapshot and frees it before
return. It returns 1 for success, including an empty or fully hidden scene.
NULL context, inactive/headless frame or snapshot allocation failure returns 0.
Allocation failure occurs before drawing. Lua retains its per-process
graph-before-command render ordering while using the shared stored descriptors
and primitive drawing operation.

`visual_process_graphics_draw` checks native pixels, z and creation ties,
priority independence, frozen/sleeping visibility, rotation/scale, zero-scale
hiding, clip/scroll scope behavior, clear and invalid frame/context calls.

A7.6af local macOS evidence: C-only (60 tests), standard/headless (177 each)
and visual (59) pass. Installed SDK probes check every new signature plus
Color/Graphic size, alignment and offsets; color-header drift is rejected by
the core probe. Native graphical ASan/UBSan exercises the primitive pass.
Linux CI and the remaining asset graph/resource integration are not certified
by these local checks.

## Rust process graphics

Core-only consumers can create `graphic::Graphic { geometry, color }`, assign
it with `actor.set_graphic(value)?`, copy it with `actor.graphic()?`, and remove
it with `actor.clear_graphic()?`. `graphic::Color` and `window::Color` refer to
the same four-byte value; the graphics feature is needed only for the window
and drawing. Graph snapshots and later edits to local copies do not mutate the
process. Cloned handles share native assignment. Dead and closed handles return
`NotFound` and `Closed`. No Rust graph object owns a process or external asset.

With the graphics feature, call `frame.process_graphics(&engine)?` after the
host's tick. It runs the native primitive pass. It does not poll futures or
add a second scheduler. Draw your other content and fade at explicit points.
Clip/scroll scopes apply, and invalid render transforms hide graphs while the
pass succeeds. Rust's `process_graphic` tests check defaults, copy independence,
shared handles, sleep/freeze, clear and dead/closed handles. The graphics pixel
fixture checks ordering, visibility, clear, invalid-scale hiding and scope
restoration. Installed core and graphical consumers exercise both boundaries.

The `Graphic` rustdoc example is runnable without a graphics backend. Migration
of public C/Lua/Rust teaching examples remains A7.8 work. Image, sprite and
animation process graphs are not available through `Graphic` yet.

A7.6ag local macOS evidence: `test-rust` and `test-rust-graphics` pass in
Debug/Release, including relocated offline core/graphics consumers, signature
and layout checks, drift diagnostics, Clippy and formatting. The new core-only
Graphic doctest passes in both profiles. `test-rust-graphics-sanitize` passes
with the new graph lifetime and process-pass pixel fixtures. Rust itself is not
sanitizer-instrumented. Linux CI and the remaining asset graph integration
remain open.

## Local sprite geometry

`mv_graphic_sprite(width, height, pivot_x, pivot_y, &shape)` creates a copied
local rectangle from cropped sprite dimensions. Dimensions are positive signed
integers. Pivots are finite double values, including fractions and points outside
the rectangle. Their magnitude must not exceed `FLT_MAX / 16`. The local center is `(width / 2 - pivot_x, height / 2 - pivot_y)`.
The importer must first adjust an original-canvas pivot for trim offsets.

The function returns 1 on success, or 0 without changing the output for invalid
dimensions, non-finite/out-of-range pivots or NULL output. It needs no image, metadata owner, process, window or
backend. Rotation and scale use the existing native collision transform. It
does not assign an image graph or retain a texture. Lua sprite descriptors now
use this same native geometry instead of computing their own local rectangle.

`sprite_geometry` checks centered, corner and external pivots, odd dimensions,
rotated/scaled overlap, signed integer extremes and failed-output preservation.
Existing headless sprite/collider and graphical asset fixtures check the Lua
migration. Installed probes check the new core signature.

Rust exposes the same operation as `collision::Shape::sprite((width, height),
(pivot_x, pivot_y))`. It returns a copied collider or InvalidArgument for
nonpositive dimensions or invalid pivots. Use the shape with existing overlap queries or local
process collider binding. Arbitrary sprite pivots are not added to primitive
process drawing. Image assignment and texture retention use the
[native process image service](images.md#native-process-image-graphs).
The Rust tests check transformed overlap, copied values, fractional pivots,
invalid numbers and integer boundaries.
The installed core-only consumer calls this constructor without graphics.


Alpha.7 fractional pivots use native ABI revision 2. The core sprite constructor
now takes double pivots, and `MvImageGraphic` stores doubles. Rebuild C and Rust
consumers against the matching SDK. Rust probes and runtime checks reject ABI
revision 1 before calling the revised operations. Rust whole-image
`Image::sprite` now returns a Result; `SpriteImage::pivot` and
`SpriteGeometry::pivot` return copied double coordinates.
