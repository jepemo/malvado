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
