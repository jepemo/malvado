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
operation; process queries are not yet implemented.

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


## Static world in C

`mv_collision_world_create()` allocates an empty, renderer-independent world.
Destroy it with `mv_collision_world_destroy()`, which also accepts `NULL`.
`mv_collision_world_add_rect(world, rect, &id)` copies an axis-aligned rectangle
and assigns a one-based ID in insertion order. It returns 1 on success, or 0
for invalid geometry, arguments, allocation failure, or when the world already
contains `INT_MAX` rectangles. On failure it leaves
the world and output unchanged. Rectangles must also satisfy the transformed
shape limits above. There is no removal operation yet.

`mv_collision_world_query(world, shape, transform, &id)` returns 1 and the
lowest inserted overlapping ID, 0 if none overlap, or -1 for invalid arguments
or shape. The output is unchanged on failure or no hit. Queries use the same
edge-contact rule as `mv_collision_overlap`. The world sorts obstacles by their
left edge and filters candidates by their bounding boxes. In the worst case it
still scans the full world. This C API does not retain the supplied shape.

```c
MvCollisionWorld *world = mv_collision_world_create();
size_t wall;
if (world && mv_collision_world_add_rect(world, (MvRect){100, 0, 10, 80}, &wall)) {
    MvCollisionShape circle = {.kind = MV_COLLISION_CIRCLE, .radius = 8};
    MvCollisionTransform position = {96, 40, 0, 1};
    size_t hit;
    int overlaps = mv_collision_world_query(world, circle, position, &hit);
    /* overlaps is 1 and hit is wall. */
    (void)overlaps;
}
mv_collision_world_destroy(world);
```

### Lua

`mv.world.create()` returns a garbage-collected world. The world owns its
rectangles. `world:add_rect({x = 100, y = 0, width = 10, height = 80})` adds
one and returns its ID. All four fields are required; unknown or invalid fields
and allocation failures raise an error. `world:overlap(shape, transform)`
returns the first overlapping ID or `nil`. It uses the same shape descriptors,
transform defaults, limits, and errors as `collision.overlap`. The `world`
namespace is not installed by `import_globals()`.

```lua
local mv = require("malvado")
local world = mv.world.create()
world:add_rect({x = 100, y = 0, width = 10, height = 80})
local hit = world:overlap(mv.collision.circle(8), {x = 96, y = 40})
```

Process queries and swept movement are not yet available.
