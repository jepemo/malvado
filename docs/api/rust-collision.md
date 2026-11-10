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

These methods do not yet derive colliders from graphics, select assets, update
native visual properties, or apply contextual movement to a process. The Lua
`world:move(process, ...)` helper already does that composition; sharing it with
C/Rust still belongs to A7.6. This explicit native geometry layer does not close
the full A7.6 graphics/input/audio/resource gate. See the
[collision contract](collision.md) for the shared math and native limits.
