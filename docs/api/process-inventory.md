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

## Lua entry forms

Both `mv.start(init, options)` and a returned game table with `init`, `width`, `height`, and `title` use the same runtime and scheduler. Initialization runs once before the first scheduled tick. Both forms use the same defaults and report initialization or process failures. They cannot be combined in one run. See [Lua processes](lua-processes.md) for the full options and errors.

`tests/start_parity.cmake` checks initialization, process ordering, and failures for both forms. `tests/start_results.cmake` checks defaults, invalid options, mixed forms, bootstrap processes, and errors. These are registered as `lua_start_parity` and `lua_start_results`.

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

## Priority comparison with BennuGD2

The [pinned BennuGD2 instance implementation](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/core/bgdrtm/instance.c) clamps priorities to -32768 through 32767 and iterates from the highest occupied priority downward. Its priority buckets insert instances at the head. The source comment on the iterator says “lower priority”, but the implementation decrements from the maximum. These observations describe the pinned source, not a claim of complete scheduler equivalence.

Malvado uses signed 64-bit priorities in C and Lua integers. It runs higher priorities first, preserves creation order for ties, and fixes the order at the start of each tick. See the [C contract](c-contract.md) and [Lua process guide](lua-processes.md). The C `schedule` test checks order and both 64-bit extremes. The headless `priorities` test checks both Lua integer extremes, tie order, and changes during a tick. Malvado does not clamp to BennuGD2's range. Other process semantics and the full reference comparison remain open.

## Signal state comparison with BennuGD2

The [pinned BennuGD2 signal implementation](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/m_proc.c) sets a live process to sleeping, frozen, or running for sleep, freeze, or wakeup. It retains separate waiting and paused flags. It also supports signal action masks and force variants. This comparison concerns those state assignments only; it does not certify the full scheduler or rendering behavior.

Malvado similarly replaces the live state when a sleep, freeze, or wakeup signal is accepted, including when the current state is already sleeping or frozen. It has no signal masks, force variants, or separate waiting and paused flags. An invalid signal leaves the process state unchanged. See the [C contract](c-contract.md) and [Lua process guide](lua-processes.md). The C `c_process` and headless `signals` tests cover repeated and cross-state signals and invalid inputs. The `visual_sleep_freeze` test covers the distinct rendering behavior.

## Tree signal comparison with BennuGD2

The pinned BennuGD2 [signal implementation](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/m_proc.c) applies tree signals to the selected instance and recursively visits its child and sibling links. Signal masks can prevent an individual state change. This observation does not establish equivalent lifetime or scheduling behavior.

Malvado snapshots the live targets for each tree signal; later children do not inherit it. C `mv_process_signal` follows ownership, while `mv_process_signal_creation_tree` follows creation history. Lua `signal_tree` uses creation history by default and can select ownership. Unlike BennuGD2, Malvado has no signal masks. See the [C contract](c-contract.md) and [Lua process guide](lua-processes.md). The C `c_process` and Lua `signal_snapshot` and `signal_relations` tests check descendants, unrelated processes, later children, and both relations. These tests verify Malvado behavior; they do not run BennuGD2.

## Creation hierarchy and lifetime comparison with BennuGD2

The pinned BennuGD2 [instance creation code](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/core/bgdrtm/instance.c#L395) records a father and links the new instance into its sibling list. Its [destructor](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/core/bgdrtm/instance.c#L529) removes the instance from its father's and siblings' links. Those functions alone do not establish all child lifetime behavior, so this comparison does not claim scheduler equivalence.

Malvado separates creation ancestry from ownership. An owned child ends with its owner; an independent child can survive its creator and retains its creator ID. C `c_process` checks natural owner return with attached and independent descendants, including a frozen child. Headless Lua `natural_cascade` checks both lifetimes, the retained creator ID, and the creation query. See the [C contract](c-contract.md) and [Lua guide](lua-processes.md).
