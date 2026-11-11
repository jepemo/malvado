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

Collider registration does not derive geometry from graphics or follow later
visual-property changes. Contextual world movement is available below. Graph
selection and automatic collider derivation still belong to A7.6. This explicit native geometry layer does not close
the full A7.6 graphics/input/audio/resource gate. See the
[collision contract](collision.md) for the shared math and native limits.

## Move a process through a world

`world.query_process(&actor, Some(shape))?` uses the actor's native x/y,
angle and scale. It returns the first overlapping world rectangle ID or `None`.
`world.move_process(&actor, Some(shape), dx, dy)?` atomically updates the
actor's x/y with the same native movement used by Lua. Pass `None` to disable
geometry and apply free movement. A registered process collider is not selected
or updated implicitly. Explicit shape selection keeps ownership and side effects
clear while the native graph service is still pending.

```rust,no_run
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
