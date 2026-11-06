# Process API inventory for alpha.6

This is the process and startup baseline from alpha.5. It is an input to the complete 2.0 inventory, not a claim that other API families are complete. The [C contract](c-contract.md), [Lua process guide](lua-processes.md), [cleanup contract](process-cleanup.md), and [resource contract](resource-lifetime.md) define the behavior.

| Area | C baseline | Lua baseline |
| --- | --- | --- |
| Runtime and start | `mv_context_create`, `mv_context_destroy`, `mv_context_runtime_id` | A returned game table or `mv.start()` |
| Process creation | `mv_process_spawn`, `mv_process_spawn_related` | `mv.process`, `mv.process.define`, `mv.process.spawn` |
| Identity and lifetime | `mv_process_ref_create`, `mv_process_ref_resolve`, `mv_process_handle_create`, `mv_process_handle_snapshot`, `mv_process_handle_destroy` | Canonical process handles; dead handles retain identity, status, and reason |
| State and control | `mv_process_status`, `mv_process_priority`, `mv_process_set_priority`, `mv_process_set_type`, `mv_process_signal`, `mv_process_kill` | `status`, `alive`, `get_priority`, `set_priority`, `signal`, `kill` |
| Queries and trees | `mv_process_find`, `mv_process_children`, `mv_process_descendants`, `mv_process_creation_children`, `mv_process_creation_descendants`, `mv_process_signal_creation_tree` | `find`, `children`, `descendants`, `signal_tree`, `kill_tree` |
| Inspection | `mv_process_snapshot`, `mv_process_lineage_snapshot`, `mv_process_inspection_snapshot`, `mv_process_inspection_capture` | `snapshot` |
| Messages and cadence | `mv_process_send`, `mv_process_receive`, `mv_process_message_count`, `mv_message_release`, `mv_cadence_step`, `mv_process_tick` | `send`, `self:recv()`, `frame`, `self.fps` |
| Resources | `mv_resource_create`, `mv_resource_transfer`, `mv_resource_retain`, `mv_resource_release`, `mv_resource_data` | Resource wrappers and ownership described in the resource contract |

C IDs are local to a context. Copyable `MvProcessRef` values validate runtime identity. An owned `MvProcessHandle` preserves a termination reason until the caller releases it. Snapshots copy engine fields and do not keep a process alive. Lua process operations use handles, not numeric IDs. Flat Lua names and namespace entries use the same implementation; see the [global export table](lua-globals.md).

Do not add `attach`, `detach`, `on_exit`, `cancel`, or a general callback-based process query as part of this baseline. Alpha.6 must compare the remaining API inventory with the pinned references and make explicit decisions. Malvado 1.0 compatibility is not a 2.0 gate.

## Alpha.6 process graphics and collision additions

These entries describe the implemented alpha.6 additions, not the complete 2.0 inventory. See [collision](collision.md), [drawing](drawing.md), and [processes](lua-processes.md) for contracts and limitations.

| Area | C | Lua |
| --- | --- | --- |
| Shapes and overlap | `mv_collision_overlap`, `mv_rect_overlaps`, `mv_rect_contains` | `collision.circle`, `collision.rect`, `collision.overlap`, `rect_overlaps`, `rect_contains` |
| Process colliders and queries | `mv_process_collider_set`, `mv_process_collider_clear`, `mv_process_collider_test`, `mv_process_collider_first`, `mv_process_collider_find` | `self.collider`, automatic `self.graph` collider, `process.collision`, `process.collisions` |
| Process graphics | Process rendering is provided by the Lua binding; native callers use the drawing API | `graphics.circle`, `graphics.rect`, supported image and sprite graphs |
| Static world | `mv_collision_world_create`, `mv_collision_world_destroy`, `mv_collision_world_add_rect`, `mv_collision_world_query`, `mv_collision_world_sweep`, `mv_collision_world_move` | `world.create`, `world:add_rect`, `world:overlap`, `world:sweep`, `world:collision`, `world:move` |

The world owns its static rectangles. Process colliders are copied into their context. Lua graphic descriptors retain their resources. Pixel masks, automatic colliders for legacy FPG sheets, and world shapes other than static rectangles are not supported.
