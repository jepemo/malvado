# Native collision and static worlds in Rust

`malvado::collision` wraps the delivered C collision API without another
collision algorithm or physics scheduler. It works without Lua, raylib, or a
window. Run the complete headless example after `mise run test-rust`:

```sh
MALVADO_SDK_PREFIX="$PWD/build-rust/Debug/relocated sdk" mise exec -- \
  cargo run --manifest-path bindings/rust/Cargo.toml -p malvado --example world
```

Create an owned World, add rectangles, and move an explicit shape/transform:

```rust
use malvado::collision::{Rect, Shape, Transform, World};
let mut world = World::new()?;
world.add_rect(Rect { x: 10.0, y: 0.0, width: 10.0, height: 10.0 })?;
let shape = Shape::circle(1.0)?;
let mut position = Transform { y: 5.0, ..Default::default() };
let contact = world.move_by(shape, &mut position, 100.0, 0.0)?;
assert_eq!(position.x, 9.0);
assert_eq!(contact.hit, Some(1));
# Ok::<(), malvado::Error>(())
```

Shape constructors validate through native geometry. `with_offset` returns a
new immutable shape. Transform defaults to position/angle zero and scale one.
`shape.overlaps(transform, other, other_transform)` tests transformed shapes;
tangency is not overlap. `Rect::contains` includes the lower edges and excludes
the upper edges. Invalid dimensions, offsets, transforms, or unrepresentable
numeric geometry return `Error::InvalidArgument`.

World rectangle IDs start at one and follow insertion order. `add_rect` copies
the rectangle and requires exclusive mutable access. `Error::WorldInsert`
preserves the C API's combined failure for invalid input, allocation failure,
or capacity exhaustion; it does not claim to distinguish those causes. Failed
insertion leaves existing geometry and later IDs unchanged.

`query` returns the first overlapping rectangle ID or None. `sweep` returns a
copied Sweep with optional hit ID, permitted fraction, and contact normal.
`move_by` updates only the supplied Transform, through the same C world move
operation. On failure it leaves that value unchanged. Initial overlap, unsupported
or unrepresentable sweeps and nonfinite displacement return InvalidArgument.
On success without a hit, fraction is one. Move along the unblocked axis in a
separate call to slide; there is no hidden movement loop in this wrapper.

World owns its native allocation and drops it once. It has no engine dependency
and can outlive a context. It cannot be sent to another thread. A completed
world can be shared through the native `Resource<World>` manager; resource
shared access does not permit later geometry mutation.

## Process colliders

`handle.set_collider(shape, transform)` copies explicit values into the native
process. Update it after changing that transform. `clear_collider()` is
idempotent for a live process. `collides_with(other)` rejects a foreign context
and excludes self-collision. `first_collision(type_id)` returns an optional
owned handle; `collisions(type_id)` returns owned handles in creation order.
Type zero selects all types. Missing colliders give no hits; sleeping and frozen
processes are included. Dead or closed handles return the corresponding error.
Failed collider updates leave the previous native collider unchanged.

Fixed registration does not follow visual-property changes. New processes now
derive colliders from selected primitive or sprite graphs by default; see the bound selection
contract below. Safe sprite assignment uses the [process sprite API](rust-atlas.md#process-sprite-assignment). This explicit native geometry layer does not close
the full A7.6 graphics/input/audio/resource gate. See the
[collision contract](collision.md) for the shared math and native limits.

## Automatic primitive and local colliders

New process handles use their assigned primitive or sprite graph for collision geometry.
Current native x/y, angle and scale apply on every collision query. Changing a
property or graph, clearing a graph, or calling contextual world movement takes
effect without rendering or another engine tick. Queries need no render frame.
Sprite assignment retains its window; primitive-only contexts need no graphics.
Animated sprite graphs use a fixed local union of all frame bounds. The visible
frame can change without changing that default collider.

Use `actor.bind_collider(Some(shape))?` to select a copied local override that
follows those same properties. `actor.bind_collider(None)?` restores selected
graph geometry. A local override survives graph clearing and replacement.
`clear_collider()` disables all selection until the next bind or fixed set.
`set_collider(shape, transform)` retains its independent fixed-transform
contract. A failed bind preserves the previous selection.

```rust
use malvado::{Engine, graphic::{Graphic, Geometry, Color}};
let engine = Engine::new()?;
let a = engine.spawn(Default::default(), |p| async move {loop {p.frame().await;}})?;
let b = engine.spawn(Default::default(), |p| async move {loop {p.frame().await;}})?;
let graphic = Graphic { geometry: Geometry::circle(2.0)?, color: Color {r:255,g:255,b:255,a:255} };
a.set_graphic(graphic)?;
b.set_graphic(graphic)?;
b.set_position(3.0, 0.0)?;
assert!(a.collides_with(&b)?);
a.clear_collider()?;
assert!(!a.collides_with(&b)?);
a.bind_collider(None)?;
assert!(a.collides_with(&b)?);
# Ok::<(), malvado::Error>(())
```

Bind validates the current selected shape/transform and returns
`InvalidArgument` for invalid geometry or numeric limits. Invalid bound source
properties make queries return `InvalidArgument`; explicit invalid targets also
return that error. Type queries skip invalid candidates. Repairing properties
restores bound geometry on the next query. Dead/closed handles return
`NotFound`/`Closed`. Sleep and freeze retain collision geometry. No shape pointer,
process resource, activation, tick or extra scheduler is retained or created.

`bound_collider.rs` shares the C/Lua golden trace and checks property/graph
changes, world writeback, local override, clear/restore, invalid-source recovery,
local offsets/rotation, fixed-transform independence and failed binds, plus
handle lifetime errors. The installed SDK consumer calls bind with and without
a local shape; the signature probe rejects an incompatible SDK.

## Use the selected collider against a world

`world.query_selected(&actor)?` uses the collider selected by native process
state. `world.move_selected(&actor, dx, dy)?` moves with that same geometry.
Primitive graphs and local overrides follow current native properties. Disabled
selection or no graph means free movement and no query hit. Nothing is retained
and no tick or future activation is added.

```rust
use malvado::{Engine, collision::{World, Rect}, graphic::{Graphic, Geometry, Color}};
let engine = Engine::new()?;
let actor = engine.spawn(Default::default(), |p| async move {loop {p.frame().await;}})?;
actor.set_graphic(Graphic {geometry:Geometry::circle(1.0)?,color:Color {r:255,g:255,b:255,a:255}})?;
actor.set_position(0.0, 5.0)?;
let mut world = World::new()?;
world.add_rect(Rect {x:10.0,y:0.0,width:10.0,height:10.0})?;
assert_eq!(world.move_selected(&actor, 100.0, 0.0)?.hit, Some(1));
assert_eq!(actor.position()?, (9.0, 5.0));
assert_eq!(world.query_selected(&actor)?, None);
# Ok::<(), malvado::Error>(())
```

Fixed registrations query their registered transform. Selected movement
translates both that transform and native process position by the permitted
displacement, preserving their offset in native double arithmetic. Ordinary
visual-property writes still leave a fixed registration independent. Native
validates both destinations before committing either, including process
position overflow when the collider destination itself is valid.

Invalid selected geometry, unsupported sweep, initial overlap or numeric input
returns `InvalidArgument`; dead/closed handles return `NotFound`/`Closed`.
Failure preserves process/collider positions and selection. The borrowed World
can outlive the engine, as before. Sleep/freeze do not block an explicit move.
Image/animation graph selection still requires the pending native asset graph
service. Existing `query_process`/`move_process` calls keep their explicit
optional-shape contract, including free movement with `None`; they do not move
fixed registrations.

`selected_world.rs` checks every selection mode, rotated/scaled geometry,
sliding, fixed-offset translation/independence, invalid-input/geometry and
overflow atomicity, missing graphs and world/handle lifetimes. The existing
C/Lua/Rust world golden trace now uses selected operations. The bounded SDK
world sample and relocated installed consumer use these native APIs.

## Move a process through a world

`world.query_process(&actor, Some(shape))?` uses the actor's native x/y,
angle and scale. It returns the first overlapping world rectangle ID or `None`.
`world.move_process(&actor, Some(shape), dx, dy)?` atomically updates the
actor's x/y with the same native movement used by Lua. Pass `None` to disable
geometry and apply free movement. A registered process collider is not selected
or updated implicitly. Explicit shape selection keeps ownership and side effects
clear while the native graph service is still pending.

```rust
use malvado::{Engine, Error};
use malvado::collision::{World, Rect, Shape};
let engine = Engine::new()?;
let actor = engine.spawn(Default::default(), |p| async move { p.frame().await; Ok(()) })?;
actor.set_position(0.0, 5.0)?;
let mut world = World::new()?;
world.add_rect(Rect { x: 10.0, y: 0.0, width: 10.0, height: 10.0 })?;
let contact = world.move_process(&actor, Some(Shape::circle(1.0)?), 100.0, 0.0)?;
assert_eq!(actor.position()?, (9.0, 5.0));
assert_eq!(contact.hit, Some(1));
# Ok::<(), Error>(())
```

Calls borrow the world and handle, copy optional geometry, and retain nothing.
They do not run a tick or poll a future. Sleep and freeze do not prevent an
explicit move. Failed movement preserves position; native errors return
`InvalidArgument`, `NotFound`, or `Closed`. Displacement must be finite and
within the native `DBL_MAX / 16` limit. World can still outlive the engine;
using a closed process handle returns `Closed`. Shape-independent movement
ignores angle/scale, including nonpositive scale.

The C, Lua and Rust contextual movement tests share the existing world golden
trace. Rust also checks scaled/rotated shape movement, free movement, sleep and
freeze, invalid displacement, initial overlap, nonpositive scale, overflow and
dead/closed handles. Installed-header probes check both signatures. The bounded
`world` SDK example uses this path; public example migration remains A7.8 work.

A7.6ai macOS host evidence: `test-rust` passes in Debug/Release with installed
signature checks, the relocated SDK consumer, Clippy and formatting. Final
bound traces pass in both profiles, and the runnable bind doctest passes in
both profiles. `test-rust-graphics-sanitize` passes with the bound collider
fixtures; native C uses ASan/UBSan and Rust itself is not instrumented. Linux CI
and asset graph/world geometry selection remain open.

A7.6ak macOS evidence: `test-rust` passes Debug/Release with the selected
C/Lua/Rust trace, installed consumer/signature checks, Clippy and formatting.
The updated bounded world sample and selected-movement doctest also pass in
both profiles. `test-rust-graphics-sanitize` passes selected-mode and fixed
translation/overflow fixtures with native ASan/UBSan; Rust itself is not
instrumented. Linux CI and native image/animation graph ownership remain open.
