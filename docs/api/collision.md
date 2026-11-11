# Rectangle collision

The [collision inventory](collision-inventory.md) maps the delivered C and Lua APIs to tests and examples.

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
operation; it does not inspect processes.

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
error. It returns a boolean. Touching edges do not overlap. This function does not query processes. Process graphics can supply automatic
colliders, as described below.

`import_globals()` installs the callable `collision` namespace. Calling
`collision(target)` queries the running process against a constructor or handle.
The geometric operations remain explicit as `mv.collision.circle`, `rect`, and
`overlap`; calling the namespace does not construct or compare shapes. The
explicit `mv.process.collision(source, target)` query remains available.


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

### Process queries in Lua

`world:collision(process, position)` tests a live process with an explicit
`collision.circle` or `collision.rect` in its `collider` field. It returns the
first overlapping obstacle ID or `nil`. It uses the process's `x`, `y`,
`angle`, and `size`, including the collider offset. A `false` collider disables
participation. A process with neither a collider nor a graph returns `nil`.
When `collider` is `nil`, a supported graphic or image supplies its shape and
pivot. Direct images use their full bounds and default to a top-left pivot. A PNG
loaded with `pivot = "center"` uses its exact center for both drawing and
automatic geometry. Explicit colliders use source-space dimensions/offsets and
receive the process scale/angle once. An atlas sprite
descriptor uses its region and metadata pivot. A graph of another type without
an explicit collider raises an error. Invalid, foreign, or dead handles and
invalid transforms raise an error. Sleeping and frozen processes participate.

The optional `position` table can replace `x`, `y`, or both for one query; it
does not move the process. Unknown fields and invalid numbers raise an error.

### Sweeping a shape

`mv_collision_world_sweep(world, shape, transform, dx, dy, &result)` sweeps a
circle or rectangle, including an oriented rectangle, along a straight displacement
against the world's axis-aligned rectangles. Lua provides
`world:sweep(shape, transform, dx, dy)`. A successful result contains `hit`,
`fraction`, `normal_x`, `normal_y`, and, for a hit, `id`. Move by the requested
displacement multiplied by `fraction`; the sweep does not modify the shape or
a process. Without a hit, the fraction is 1 and the normal is zero. Equal-time
hits select the lowest obstacle ID. At an edge, motion into an obstacle hits at
fraction zero; motion away or parallel to it is allowed. A path that only
grazes an edge or corner is not a hit. The endpoint can report a hit at
fraction 1. Floating-point rounding can affect contact at rotated edges.

The C function returns 1 for a valid sweep and -1 for invalid input, initial
overlap, or an unsupported shape; on error it leaves the result unchanged. Lua
raises an error in those cases. Displacements must be finite and each component
must not exceed `DBL_MAX / 16` in magnitude. The start and end transforms must
meet the shape limits above. For a circle, the radius divided by the largest displacement component,
coordinate difference from a candidate rectangle, or candidate dimension must
be at least `8 * sqrt(DBL_EPSILON)`. Values below that limit are rejected to
avoid unreliable corner calculations. Floating-point rounding can affect
near-tangent contacts.

```lua
local box = mv.collision.rect(2, 2)
local result = world:sweep(box, {x = 0, y = 40}, 200, 0)
local allowed_x = 200 * result.fraction
```


### Move a process

`world:move(process, dx, dy)` sweeps its explicit circle or rectangle collider,
then updates `x` and `y` by the permitted displacement. It returns a table with
`hit`, `moved_x`, `moved_y`, `normal_x`, `normal_y`, and an obstacle `id` on
impact. A disabled collider or a process without a collider or graphic moves
freely. A supported graphic or image supplies a collider when no explicit collider is
set. Other graphic types require an explicit collider. The operation does not slide.
An invalid process or transform, initial overlap, or a sweep outside the
numeric limits raises an error. A process that starts inside an obstacle must be
repositioned before it can use this operation.

In C, `mv_collision_world_move(world, shape, &transform, dx, dy, &result)`
updates the caller's transform on success. It returns 1 on success and -1 on
error, leaving both outputs unchanged on error. To avoid rounding into an
obstacle, it can reduce a hit fraction by a few representable steps. If it
cannot find a non-overlapping destination within eight steps, it returns an
error. Lua reports the actual displacement after this adjustment.

```lua
local result = world:move(player, 4, 0)
if result.hit then print("Wall", result.id) end
```


## Process colliders in C

Include `malvado/collision.h` and use a live `MvProcessRef` from the same
context. `mv_process_collider_set(ctx, ref, shape, transform)` copies the
shape and transform into that process. Call it again when the C process moves
or changes its collider. `mv_process_collider_clear(ctx, ref)` disables the
collider and is safe to repeat. Invalid geometry returns `MV_INVALID_ARGUMENT`
without changing the old collider. A dead process returns `MV_NOT_FOUND`; a
foreign reference returns `MV_INVALID_ARGUMENT`. Process termination removes
the collider with the process. No function in this group allocates.

`mv_process_collider_test(ctx, a, b, &hit)` compares two live processes and
writes 1 or 0. A process does not collide with itself; a missing collider gives
0. `mv_process_collider_first(ctx, source, type, &ref)` writes the first
matching reference, or a zero reference if there is none.
`mv_process_collider_find(ctx, source, type, buffer, capacity, &total)`
copies matching references in creation order, excluding the source. Type 0
selects all types. It writes the total even when the buffer is too small and
accepts a null buffer when capacity is zero. Both include sleeping and frozen
processes and exclude dead processes. A bad argument or dead source returns an
error without changing the outputs. `mv_process_info(ctx, ref, &info)` copies a
live process record for a reference and leaves the output unchanged on error.
The implementation scans the process list in creation order and rejects distant
candidates with cached transformed bounds before the narrow phase. Updates
refresh the bounds with the collider. This linear scan can still be expensive
in large or dense scenes; see [benchmarks](../../tools/benchmarks/README.md). Lua process colliders synchronize
with this same C registry when their relevant fields change. Lua fields are the
source of truth for Lua-owned process colliders; do not overwrite those entries
from C, because the next Lua field update replaces them. Native processes use
the C registration functions.

```c
MvCollisionShape circle = {.kind = MV_COLLISION_CIRCLE, .radius = 10};
MvCollisionTransform transform = {100, 100, 0, 1};
if (mv_process_collider_set(ctx, player, circle, transform) == MV_OK) {
    MvProcessRef hits[8];
    size_t count;
    if (mv_process_collider_find(ctx, player, 0, hits, 8, &count) == MV_OK) {
        /* The first min(count, 8) entries contain process references. */
    }
}
```


## Process-to-process queries in Lua

`mv.process.collision(source, target)` returns the first colliding process
handle or `nil`. `mv.process.collisions(source, target)` returns a fresh table
of matching handles in creation order. The target is a process constructor or a
specific process handle. The source is excluded, and a specific dead target
does not match. Sleeping and frozen processes participate. Handles are the same
canonical handles returned by process creation and inspection. Touching without
overlap is not a collision.

```lua
local enemy = process.collision(self, Enemy)
if enemy then enemy.health = enemy.health - 1 end
```

The source must be live. Invalid or foreign handles raise an error. An invalid
source collider or transform raises an error, as does an invalid explicitly
selected live target. When selecting by type, processes with invalid colliders
or transforms are excluded until corrected. A `false` collider, or no graph
and no collider, excludes the process. Assignments to `x`, `y`, `angle`,
`size`, `graph`, and `collider`, and `world:move`, update the native collider
immediately. The first-hit query does not allocate a result table. Both queries
work without a window.

A legacy image drawn by legacy `frame()` uses a centered automatic collider.
A legacy FPG sheet needs an explicit collider; its automatically derived
collider is not supported.


## Contextual process and world calls

```lua
require("malvado").import_globals()
local world = world_create()
world:add_rect({x = 300, y = 100, width = 40, height = 280})
local Player = process(function(self)
  self.world = world
  self.graph = require("malvado").graphics.circle(16)
  local result = world_move(4, 0)
  if result.hit then print("Wall", result.id) end
end)
start(function() Player({x = 100, y = 200}) end)
```

`world_create()` is the same function as `mv.world.create()`. A process can
retain a world in `self.world`, including in its defaults. This association
does not move it, register it as an obstacle, or make x/y assignments physical.
The reference keeps the world alive while the process fields retain it.
Changing or clearing the field changes only future contextual calls.

`world_move(dx, dy)` resolves the actual running process and its `world` field.
It delegates to `world:move(self, dx, dy)` and returns the same hit table,
actual displacement, normal, and optional obstacle ID. The explicit
`world_move(world, player, dx, dy)` works without a running process and ignores
any contextual world. `mv.world.move` and `mv.world_move` are the same function.
The method remains available. All forms use the same native sweep and do not slide.

`collision(target)` returns the same canonical handle or `nil` as
`mv.process.collision(self, target)`, with the same creation ordering,
self-exclusion, sleeping/frozen participation, and dead-target behavior.
For an explicit source, use the existing namespace query.

Implicit calls fail outside the scheduled process coroutine, including inside
a nested user coroutine. A missing or invalid `self.world` raises a useful
error on movement. Explicit movement validates its world and process handles
instead. Invalid calls do not move the process. A disabled collider (`false`),
an automatic collider (`nil`) and an explicit collider keep their existing
behavior in every form. Worlds and descriptors remain garbage-collected; no
new native resource or scheduler is introduced by these wrappers.

## Contextual process movement

`mv_process_world_query(ctx, process, world, shape, &id)` uses native process
x/y, angle and scale. It writes a one-based world ID, or zero for no overlap.
`mv_process_world_move(ctx, process, world, shape, dx, dy, &result)` uses the
same properties and commits the permitted x/y atomically. Both borrow `world`
and the optional shape for this call. Passing NULL for the shape disables world
collisions: query returns zero and move applies the full displacement.
No graph or registered process collider is selected implicitly.

Both operations require a live owner-thread context and a context-bound live
process. Foreign references, NULL world/output, invalid geometry or numeric
limits return `MV_INVALID_ARGUMENT`. Dead processes return `MV_NOT_FOUND`.
The move displacement must be finite and at most `DBL_MAX / 16` in magnitude.
Initial overlap and unsupported sweep transforms follow the existing world
rules. Failure preserves process position and output. Sleep and freeze do not
prevent an explicit host move. These calls do not tick, allocate, retain data,
change angle/scale, or update explicitly registered C collider transforms.

Lua `world:collision` without a hypothetical position and `world:move` use these
operations for native numeric properties. Lua still selects explicit or
graph-derived geometry and refreshes its derived collider after movement.
Hypothetical query positions and local invalid visual overrides retain their
existing conversion/error path. The core world geometry remains shared in all
paths. `process_world` checks identity, atomic errors, fast/slide movement,
rotation/scale, no-shape movement and numeric boundaries. `alpha7_world_parity`
compares the contextual C/Lua trace with the existing golden fixture.

A7.6ab local macOS verification: `mise run test-c` (58 tests),
`mise run test` and `mise run test-headless` (173 tests each), and
`mise run test-visual` (57 tests) pass. This is host evidence, not Linux CI
acceptance or automatic graph selection in C/Rust.

## Bound process colliders

New native processes select their primitive graph for collision geometry.
Assigning or replacing a primitive graph, changing x/y, angle or scale, clearing
a graph, or moving through a world becomes visible on the next process collision
query. No tick, render or explicit collider resynchronization is required.
No graph means no collider. Sleeping and frozen processes remain queryable.

`mv_process_collider_bind(ctx, reference, override)` selects a copied local
shape when `override` is non-NULL. That shape follows the native process x/y,
angle and scale and survives graph replacement/clearing. Pass NULL to restore
primitive graph selection. Binding validates the currently selected shape and
transform before changing selection. Invalid shape/transform or foreign/NULL
context returns `MV_INVALID_ARGUMENT`; a dead process returns `MV_NOT_FOUND`.
Errors preserve the previous selection. Inputs are borrowed only for the call.
No allocation, retained pointer, resource ownership or scheduler is introduced.

`mv_process_collider_set` preserves its previous fixed shape/transform behavior:
later visual-property changes do not move that collider. `clear` disables all
collider selection until `set` or `bind`. It remains idempotent. This revises
the earlier alpha.7 primitive default, which did not derive C/Rust colliders.
Use clear to disable automatic collision, or fixed set for an independent
transform. These choices keep old explicit registrations stable.

Queries recompute bounds for bound colliders from current native state.
Nonpositive scale or unsupported numeric geometry makes a bound collider
invalid, without keeping stale geometry. Pair queries with an invalid source or
explicit target return `MV_INVALID_ARGUMENT` without changing output. Type
queries reject invalid sources and skip invalid candidates, as in Lua. Repairing
the native properties restores a bound collider on the next query.

Lua uses bound selection for primitive graphs and local overrides with native
numeric properties. Local invalid visual values, numeric-string conversions and
image/sprite/animation geometry keep their existing binding handling. Lua still
owns its `self.collider` selection and invalid-visual metadata. Direct native
changes to a bound Lua process's graph/properties are observable by C collision
queries before another Lua activation or query.

`bound_collider` and the C/Lua/Rust golden trace cover graph-derived defaults,
position/rotation/scale, copied local override, disable/restore, graph clear and
replacement, world movement, sleep/freeze and invalid targets. Native tests also
check invalid-source output preservation, foreign/dead/NULL errors, failed bind
atomicity and the fixed-transform contract. The mixed graph host fixture checks
immediate C queries after native replacement and clear, with no Lua update.
World operations that accept an explicit optional shape retain their existing
contract: NULL still means free movement, not selected collider geometry.

A7.6ah local macOS evidence: C-only (61 tests), standard/headless (179 each)
and visual (59) pass. The updated mixed C/Lua host and final golden trace also
pass in targeted checks. Installed-header probes verify the new bind signature.
Linux CI and asset graph collider derivation are not certified by these checks.

## World operations with selected geometry

`mv_process_world_query_selected(ctx, ref, world, &id)` queries the collider
selected by the process. `mv_process_world_move_selected(ctx, ref, world,
dx, dy, &result)` moves with the same selection. Primitive graph and local
bound selections use current native visual properties. Disabled selection or
an absent graph gives no query hit and applies free movement.

Fixed registrations use their explicit collider transform. A selected move
translates both that collider and the native process origin by the permitted
displacement. Their existing offset is preserved in native double arithmetic.
Angle and scale remain unchanged. Manual property writes still leave fixed
colliders independent. This explicit move contract keeps selected world queries
and process collision queries consistent when collider and process origins differ.

The world and process must be live on the owner thread. No world/shape pointer
is retained; no allocation, callback, tick or activation occurs. Query writes
a one-based rectangle ID or zero. Invalid selection, unsupported sweep geometry,
initial overlap, NULL inputs or numeric overflow returns `MV_INVALID_ARGUMENT`.
Dead processes return `MV_NOT_FOUND`; foreign references are invalid. Movement
validates both process and collider destinations before committing either one.
Failures preserve positions, selection and outputs. Displacement obeys the
existing finite `DBL_MAX / 16` limit. A valid selected collider can still move
while the process sleeps or freezes.

Lua `world:collision` at the current position and `world:move` use selected
geometry after the existing binding validation/synchronization. Hypothetical
query positions and local invalid/numeric-string overrides keep their existing
conversion path. Asset shapes remain supplied by the Lua binding; this step
does not add native image/animation graph storage.

The older explicit-shape process/world calls keep their contract. In those
calls NULL still means free movement, and fixed registrations are not changed.
`selected_world` checks graph/local/disabled/fixed selection, scaled/rotated
geometry, sliding, frozen movement, offset preservation, initial overlap,
invalid geometry/input and destination-overflow atomicity, NULL/foreign/dead
references and unchanged output buffers. The existing world golden trace now
runs the selected native path in C and Lua.

A7.6aj macOS evidence: C-only (62 tests), standard/headless (180 each) and
visual (59) pass. Final selected-world and golden parity checks pass after
adding absent-graph and output-buffer cases. Installed-header probes check
both selected signatures. Linux CI and native asset graph ownership remain open.
