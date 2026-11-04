# Rectangle collision

Use axis-aligned rectangles to test simple game geometry without a window. These functions do not find processes, derive bounds from sprites, or test pixel masks. The caller chooses the coordinate space and updates bounds as objects move. The C API also supports transformed circles and rectangles, described below.

A rectangle uses `x`, `y`, `width`, and `height`. All values must be finite; width and height must be positive. The right and bottom coordinates must be finite and greater than the origin after floating-point addition. The left and top edges are included; the right and bottom edges are excluded. Rectangles that only touch at an edge do not overlap.

## Lua

```lua
local mv = require("malvado")
if mv.rect_overlaps(10, 10, 16, 16, 20, 20, 8, 8) then
    print("hit")
end
if mv.rect_contains(10, 10, 16, 16, 12, 12) then
    print("inside")
end
```

Both functions return a boolean and raise an error for invalid arguments. They are available in headless mode and do not access process state.

## C

Include `malvado/collision.h` and link `malvado_core`. Both functions return 1 for a hit, 0 for no hit, and -1 for invalid input. Values are passed by value; there is no allocation or retained state.

```c
MvRect player = {10, 10, 16, 16};
MvRect target = {20, 20, 8, 8};
int hit = mv_rect_overlaps(player, target);
int inside = mv_rect_contains(player, 12, 12);
```


## Transformed shapes in C

`mv_collision_overlap` tests two circles or oriented rectangles. Construct an
`MvCollisionShape` with `MV_COLLISION_CIRCLE` and a positive `radius`, or with
`MV_COLLISION_RECT` and positive `width` and `height`. Its `offset_x` and
`offset_y` give the local center relative to the origin. Pass an
`MvCollisionTransform` with world `x` and `y`, clockwise `angle` in degrees,
and positive uniform `scale`. The offset is scaled and rotated with the shape.

The function returns 1 for overlap, 0 for separation or edge contact, and -1
for invalid values. Values must be finite. World centers and bounding
half-extents must not exceed `DBL_MAX / 16` in magnitude. Floating-point
rounding can affect contact at rotated edges. Shapes are values owned by the
caller; no allocation or renderer is involved. This is a low-level geometry
operation; process queries and a collision world are not yet implemented.

## Reusable shapes in Lua

Use `mv.collision` to create shapes and test them without a window:

```lua
local collision = require("malvado").collision
local circle = collision.circle(10)
local box = collision.rect(40, 20, {offset_x = 20, offset_y = 10})
if collision.overlap(circle, {x = 12, y = 10}, box, {x = 0, y = 0}) then
    print("Overlap")
end
```

`circle(radius, options)` and `rect(width, height, options)` return immutable,
reusable descriptors. Dimensions must be positive and finite. The optional
`offset_x` and `offset_y` specify the shape center relative to its origin; both
default to zero. For a top-left rectangle origin, use half the width and height
as the offsets. Invalid dimensions, offsets, or unknown options raise an error.
Lua retains descriptors while they are referenced. They can be shared in
process defaults without sharing mutable tables.

`overlap(shape_a, transform_a, shape_b, transform_b)` uses the C geometry
operation. Each transform is `nil` for the identity or a table with `x`, `y`,
`angle`, and `scale`; these default to zero, zero, zero degrees, and one. All
values must be finite and scale must be positive. Invalid shapes, transforms,
unknown fields, or values outside the C operation's documented limits raise an
error. It returns a boolean. Touching edges do not overlap. This function does
not query processes or automatically attach colliders to them.

The `collision` namespace is not installed by `import_globals()`. Use
`require("malvado").collision`. A global `collision(...)` shorthand is not
provided: process selection and shape geometry have different arguments, and a
single global name would make their meaning ambiguous. Process collision queries
remain planned in the `process` namespace.
