# Collision API baseline for alpha.6

This map records the collision API delivered in C and Lua. The [collision
contract](collision.md) defines coordinates, edge contact, errors, ownership,
and supported sweep shapes. The [collision plan](../versions/2.0.0-alpha.6-collision.md)
records its automated acceptance evidence. This map does not select future
collision features or certify compatibility with historical APIs.

| Area | C | Lua | Evidence |
| --- | --- | --- | --- |
| Rectangle tests | `mv_rect_overlaps`, `mv_rect_contains` | `mv.rect_overlaps`, `mv.rect_contains` | `collision`, `lua_collision` |
| Transformed shapes | `mv_collision_overlap` | `mv.collision.circle`, `mv.collision.rect`, `mv.collision.overlap` | `collision`, `lua_collision` |
| Process colliders | `mv_process_collider_set`, `mv_process_collider_clear`, `mv_process_collider_test`, `mv_process_collider_first`, `mv_process_collider_find` | `self.collider`, `mv.process.collision`, `mv.process.collisions` | `process_collision`, `lua_process_collisions`, `mixed_collision` |
| World and movement | `mv_collision_world_create`, `mv_collision_world_destroy`, `mv_collision_world_add_rect`, `mv_collision_world_query`, `mv_collision_world_sweep`, `mv_collision_world_move` | `mv.world.create` and `world:add_rect`, `world:overlap`, `world:sweep`, `world:collision`, `world:move` | `collision`, `lua_collision`, `lua_world_process`, `collision_allocation` |

The C API accepts copied values and explicit transforms. Lua manages descriptor
and world lifetimes and can derive process colliders from supported graphics.
C world operations use explicit shapes; Lua world operations can also use a live
process. Neither API implements pixel masks. The [contract](collision.md) includes C examples. The [Lua collision example](../../examples/collision/main.lua), and
[world example](../../examples/world-collision/main.lua) show supported use.

## Collision types

| C types | Meaning and Lua relationship |
| --- | --- |
| `MvRect` | Rectangle value; Lua rectangle helpers accept coordinates. |
| `MvCollisionKind`, `MvCollisionShape` | Native shape kind and value; Lua creates immutable circle or rectangle descriptors. |
| `MvCollisionTransform` | Native position, angle, and scale value; Lua accepts transform tables or process fields. |
| `MvCollisionWorld` | Opaque C-owned world; Lua world objects manage their lifetime. |
| `MvCollisionSweep` | Copied C sweep result; Lua returns a result table. |

## C collision constants

| C enum | C constants | Lua relationship |
| --- | --- | --- |
| `MvCollisionKind` | `MV_COLLISION_CIRCLE`, `MV_COLLISION_RECT` | Lua constructs descriptors with `mv.collision.circle` or `mv.collision.rect`. |

`public_enum_map` checks the C enum and constant names, not numeric values.

`collision_api_map` checks public function and type names against the header. It
does not check fields, constants, semantics, or whether every C operation has a
Lua equivalent.


## Contextual Lua wrappers

`collision(target)` delegates to `mv.process.collision` for the running process.
`world_create` aliases `mv.world.create`; `world_move` and `mv.world.move` add
contextual/explicit access to the existing world movement implementation.
These wrappers add no native symbols. See the [context contract](collision.md#contextual-process-and-world-calls),
`tests/contextual_world.lua`, and `tests/contextual_world_start.lua` for results,
errors, coroutine isolation, collider overrides, retention and both entry forms.
The collision/world examples exercise contextual queries and movement; explicit
method/query regressions remain in the existing collision and world tests.
