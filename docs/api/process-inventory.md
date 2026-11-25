# Process API inventory for alpha.6

This is the process and startup baseline from alpha.5. It is an input to the complete 2.0 inventory, not a claim that other API families are complete. The [C contract](c-contract.md), [Lua process guide](lua-processes.md), [cleanup contract](process-cleanup.md), and [resource contract](resource-lifetime.md) define the behavior.

| Area | C baseline | Lua baseline |
| --- | --- | --- |
| Native version discovery | `mv_abi_version`, `mv_version_string` | C SDK queries used for [native ABI and release checks](rust-support.md). |
| Game asset paths | `mv_context_create_with_game_dir`, `mv_context_game_dir`, `mv_set_assets_dir`, `mv_get_assets_dir`, `mv_resolve_asset_path` | `mv.set_assets_dir`, `mv.get_assets_dir`, and the declarative `assets_dir` field |
| Runtime and start | `mv_context_create`, `mv_context_destroy`, `mv_context_runtime_id`, `mv_context_advance`, `mv_context_ticks`, `mv_context_dropped_seconds` | A returned game table or `mv.start()` |
| Process creation | `mv_process_spawn`, `mv_process_spawn_related` | `mv.process`, `mv.process.define`, `mv.process.spawn` |
| Identity and lifetime | `mv_process_ref_create`, `mv_process_ref_resolve`, `mv_process_handle_create`, `mv_process_handle_snapshot`, `mv_process_handle_destroy` | Canonical process handles; dead handles retain identity, status, and reason |
| State and control | `mv_process_status`, `mv_process_count`, `mv_process_priority`, `mv_process_set_priority`, `mv_process_set_type`, `mv_process_signal`, `mv_process_kill` | `status`, `alive`, `get_priority`, `set_priority`, `signal`, `kill` |
| Queries and trees | `mv_process_find`, `mv_process_children`, `mv_process_descendants`, `mv_process_creation_children`, `mv_process_creation_descendants`, `mv_process_signal_creation_tree` | `find`, `children`, `descendants`, `signal_tree`, `kill_tree` |
| Native position | `mv_process_position`, `mv_process_set_position` | `self.x`, `self.y`; shared numeric position, Lua-derived colliders and graph rendering. |
| Visual angle/scale | `mv_process_angle`, `mv_process_set_angle`, `mv_process_scale`, `mv_process_set_scale` | `self.angle`, `self.size`; shared finite values, local invalid overrides. |
| Visual order | `mv_process_z`, `mv_process_set_z`, `mv_process_draw_order` | `self.z`; shared native renderer order |
| Inspection | `mv_process_info`, `mv_process_snapshot`, `mv_process_lineage_snapshot`, `mv_process_inspection_snapshot`, `mv_process_inspection_capture` | `snapshot` |
| Messages and cadence | `mv_process_send`, `mv_process_receive`, `mv_process_message_count`, `mv_message_release`, `mv_cadence_step`, `mv_process_tick` | `send`, `recv()` or `recv(handle)`, `frame`, `self.fps`, startup `fps`, `set_fps` |
| Resources | `mv_resource_create`, `mv_resource_transfer`, `mv_resource_retain`, `mv_resource_release`, `mv_resource_data` | Resource wrappers and ownership described in the resource contract |

The table maps the public functions in `malvado.h`, not every future 2.0 API.
`process_api_map` checks C function and type-name coverage when Python is available. It does
not establish semantic parity: Lua has no general native clock or resource
transfer API, and native process type IDs differ from Lua process definitions.

## Process types and values

This map covers the public types in `malvado.h`. The C definitions and detailed
contracts are in the [header](../../src/include/malvado/malvado.h) and
[C contract](c-contract.md); Lua behavior is in the [process guide](lua-processes.md).
The check detects missing or extra type names, not changes to fields or semantics.

| C types | Meaning and Lua relationship |
| --- | --- |
| `MvContext`, `MvCadence` | Context and cadence state are C-managed. Lua uses the runner and `self.fps`. |
| `MvResult` | C result codes; Lua operations generally report errors or return `nil` as documented per operation. |
| `MvProcessId`, `MvRuntimeId`, `MvProcessRef`, `MvProcessHandle` | C identity and handle types. Lua uses process handles and exposes IDs and snapshot runtime identity. |
| `MvProcessStatus`, `MvEndReason`, `MvSignal` | C enums. Lua exposes state and reason strings, and signal strings and `SIGNAL_*` constants. |
| `MvStepResult`, `MvProcessStep`, `MvProcessDestroy` | Native callback result and callbacks; Lua uses process coroutines and Lua cleanup. |
| `MvProcessDrawInfo`, `MvProcessInfo`, `MvProcessHandleInfo`, `MvLineageInfo`, `MvInspectionHeader` | Copied C inspection values. Lua exposes process handles and snapshot tables, not these structs. |
| `MvMessageType`, `MvMessage` | Native message tags and values. Lua exchanges primitive values and binary strings with C; Lua-only objects stay in the VM. |
| `MvResource`, `MvResourceDestroy` | Native resource and destructor. Lua wrappers retain resources but do not expose C ownership transfer. |

## C process constants

| C enum | C constants | Lua relationship |
| --- | --- | --- |
| `MvResult` | `MV_OK`, `MV_INVALID_ARGUMENT`, `MV_OVERFLOW`, `MV_NOT_FOUND`, `MV_OUT_OF_MEMORY`, `MV_BUSY`, `MV_PROCESS_ERROR`, `MV_QUEUE_FULL`, `MV_QUEUE_EMPTY`, `MV_BUFFER_TOO_SMALL` | Lua uses operation-specific errors or results; it does not expose these C codes. |
| `MvProcessStatus` | `MV_PROCESS_RUNNING`, `MV_PROCESS_SLEEPING`, `MV_PROCESS_FROZEN` | Lua uses `running`, `sleeping`, and `frozen` strings; a dead handle reports `dead`. |
| `MvEndReason` | `MV_END_KILLED`, `MV_END_RETURNED`, `MV_END_ERROR` | A dead Lua handle reports `killed`, `returned`, or `error`. |
| `MvSignal` | `MV_SIGNAL_SLEEP`, `MV_SIGNAL_FREEZE`, `MV_SIGNAL_WAKEUP`, `MV_SIGNAL_KILL` | Lua uses matching signal strings and `SIGNAL_*` string constants. |
| `MvStepResult` | `MV_STEP_YIELD`, `MV_STEP_DONE`, `MV_STEP_ERROR` | C callback results have no direct Lua enum; Lua yields, returns, or raises an error. |
| `MvMessageType` | `MV_MESSAGE_BOOLEAN`, `MV_MESSAGE_INTEGER`, `MV_MESSAGE_NUMBER`, `MV_MESSAGE_BYTES`, `MV_MESSAGE_OPAQUE` | Lua exchanges primitive values and byte strings with C; Lua objects stay in the VM. |

`public_enum_map` checks C enum and constant names, not their numeric values or
semantics. See the [C contract](c-contract.md) and [Lua guide](lua-processes.md)
for errors, lifecycle, and message ownership.

## C creation, identity, and control evidence

The [C control example](../../examples/c/control/README.md) demonstrates the
separate ownership and creation relations, process state, priority, type, and
a retained handle. `c_control_example` runs it in-tree and `installed_core_sdk`
builds and runs it against a relocated SDK. The
[C process example](../../examples/c/processes/README.md) shows callback and
destructor lifetime. The [C contract](c-contract.md#processes-and-signals) defines errors,
ownership, and lifetime; the [Lua guide](lua-processes.md) describes handles and
process definitions.

| C symbols | Behavior evidence and Lua relationship |
| --- | --- |
| `mv_process_spawn`, `mv_process_spawn_related` | `c_process` checks creation, invalid relations, cleanup, and owned and independent descendants. Lua `process_lifecycle` and `natural_cascade` check creation and lifetime. |
| `mv_process_ref_create`, `mv_process_ref_resolve` | `c_process` and `c_process_handle` check foreign runtimes, ended identities, and unchanged outputs. Lua uses canonical handles instead of these C values. |
| `mv_process_handle_create`, `mv_process_handle_snapshot`, `mv_process_handle_destroy` | `c_process_handle` checks allocation failure, invalid inputs, termination reasons, and handles after context destruction. Lua `process_lifecycle` checks retained handle behavior. |
| `mv_process_status`, `mv_process_count` | `c_process` checks live counts, signal states, invalid inputs, and unchanged outputs. Lua `signals` checks state changes. |
| `mv_process_priority`, `mv_process_set_priority`, `mv_process_set_type` | `c_process` checks type, priority, invalid inputs, and unchanged output; `schedule` checks ordering and integer limits. Lua `priorities` checks its priority API. Native type IDs have no direct Lua equivalent. |
| `mv_process_signal`, `mv_process_signal_creation_tree`, `mv_process_kill` | `c_process` checks invalid signals, both tree relations, dead references, and cleanup. Lua `signals`, `signal_relations`, and `natural_cascade` check state and lifetime. |

## C query and inspection evidence

The [C inspection example](../../examples/c/inspection/README.md) runs without
Lua or a graphics backend. It compares ownership and creation relations, then
captures process and lineage records. `c_inspection_example` runs it in-tree;
`installed_core_sdk` also builds and runs it against the relocated SDK. The
[C contract](c-contract.md#inspect-a-snapshot) defines buffer rules, errors,
and record lifetime. The [Lua inspection example](../../examples/inspection/README.md)
shows the Lua snapshot API.

| C symbols | Behavior evidence and Lua relationship |
| --- | --- |
| `mv_process_info` | `process_collision` and `c_query_allocation` check live records, invalid references, and allocation-free access. Lua uses process handles. |
| `mv_process_snapshot`, `mv_process_find` | `c_process` checks filtering, ordering, capacities, and invalid outputs. Lua `snapshot` and `find` are checked by `inspection`. |
| `mv_process_children`, `mv_process_descendants` | `c_process` checks ownership, filters, truncation, and invalid roots. `signal_relations` checks Lua relations. |
| `mv_process_creation_children`, `mv_process_creation_descendants` | `c_process` and `c_long_session` check live and ended creators and cleanup. Lua uses a creation relation option; `lineage_snapshot` and `signal_relations` check it. |
| `mv_process_lineage_snapshot` | `c_process` and `c_long_session` check retained and pruned creation records. Lua `snapshot` reports lineage information, checked by `lineage_snapshot`. |
| `mv_process_inspection_snapshot`, `mv_process_inspection_capture` | `c_process` checks capacities, unchanged output, and capture identity and tick. Lua `snapshot` returns its own copied table; it does not expose these C buffer functions. |

These checks cover the delivered C query and inspection functions. The full
2.0 API inventory remains open.

## Lua creation, identity, control, and inspection evidence

The [Lua process control example](../../examples/process-control/README.md)
runs headless and demonstrates every operation in the table. The
[inspection example](../../examples/inspection/README.md) shows snapshots and
flat global functions. `lua_process_control_example` and `inspection_example`
run these examples. The [process guide](lua-processes.md) defines the contracts;
the [global](lua-globals.md) and [namespace](lua-namespaces.md) inventories
map their equivalent exported names.

| Lua symbols | Behavior evidence |
| --- | --- |
| `process`, `process.define`, `spawn`, `process.spawn` | [process_defaults](../../tests/process_defaults.lua) checks definition defaults and invalid fields; [process_attachment](../../tests/process_attachment.lua) and [natural_cascade](../../tests/natural_cascade.lua) check attached and independent lifetime and cleanup. `lua_spawn_allocation` checks allocation failures. |
| `status`, `alive`, process handles | [inspection](../../tests/inspection.lua) checks canonical handles, invalid numeric IDs, dead handles, and released game fields. [process_lifecycle](../../tests/process_lifecycle.lua) checks copied fields and cleanup. |
| `set_priority`, `get_priority` | [priorities](../../tests/priorities.lua) checks integer boundaries, ordering, changes during a tick, and invalid or dead handles. |
| `signal`, `signal_tree`, `kill`, `kill_tree` | [signals](../../tests/signals.lua), [signal_relations](../../tests/signal_relations.lua), and [signal_snapshot](../../tests/signal_snapshot.lua) check state transitions, invalid signals, both relations, later children, and termination. |
| `find`, `children`, `descendants` | [inspection](../../tests/inspection.lua), [signal_relations](../../tests/signal_relations.lua), and [lineage_snapshot](../../tests/lineage_snapshot.lua) check ordering, filters, invalid options, dead ancestors, and canonical handles. |
| `snapshot` | [inspection](../../tests/inspection.lua) and [lineage_snapshot](../../tests/lineage_snapshot.lua) check copied state, visibility, runtime identity, lineage, and ended processes. |

Each flat operation above is also available through `process`; `process.define`
is namespace-only. `lua_global_inventory` and `lua_namespace_inventory` check
the names and alias identities. The checks concern the delivered API, not the
unselected future 2.0 inventory. Process collision, cadence, and messaging
are covered by their separate API reviews.

## Lua process context

An explicit process argument is a handle from the same runtime. Each operation
has its own liveness rules. In the table below, outside calls include setup with
valid arguments and any required assets or graphics support.

| Operation | Implicit process | Explicit process and outside calls | Ergonomics, effects, and validation |
| --- | --- | --- | --- |
| `frame()` | Scheduled coroutine | No explicit handle or outside call | Yielding the caller is the natural form. It checks thread identity and yieldability. The main path does not separately reject a dead process slot. |
| `collision(target)` | Scheduled source | The namespace forms `process.collision(source, target)` and `process.collisions(source, target)` accept explicit sources and work outside | The shorthand is useful inside a process. It shares the first-hit query with the explicit form, which supports a different source. A dead target is a miss. The source must be live. |
| `world_move(dx, dy)` | Scheduled process and its world | `world_move(world, process, dx, dy)` works outside | Both forms are useful. They move the selected live process. The explicit form also chooses the world. |
| `world_collision(position?)` or `world_collision(world, process, position?)` | Scheduled process and its `world` field. | Live process handle and world. Works outside. | Either form can query a hypothetical position without moving the process. |
| `graph(id)` | Scheduled process supplies `self.file` | No process handle. `graph(library, id)` works outside. | The contextual form is already short. The library form supports setup and reuse. Selection returns a descriptor. |
| `animation(sequence, options?)` | Scheduled process supplies `self.file` | No process handle. `animation(library, sequence, options?)` works outside. | Both forms are useful. Selection returns a descriptor. Assign the descriptor to a graph to start playback. |
| `recv()` or `recv(handle)` | Scheduled recipient when omitted | Live handle. The explicit form works outside. | The shorthand fits the usual self-receive. The explicit form selects another queue. Both remove one message on success and validate selection before reading. |
| `animation_elapsed()` or `animation_elapsed(handle)` | Scheduled process when omitted | Live handle. The explicit form works outside. | The shorthand fits local playback checks. The explicit form inspects another process. Both read the clock without changing playback. |
| `animation_finished()` or `animation_finished(handle)` | Scheduled process when omitted | Live handle. The explicit form works outside. | Checks whether a non-looping animation reached its total duration. |
| `send(handle, value)` | None | Live recipient. Works outside. | Keep the destination visible. An implicit recipient could enqueue to the wrong process. |
| `kill(handle)` | None | Explicit target. Works outside. A dead target returns false. | The shortcut for `signal(handle, "kill")` makes termination easy to read. Keep the target visible because attached descendants can also end. |
| `kill_tree(handle)` | None | Explicit root. Works outside. | The shortcut selects the creation relation. The current wrapper uses only the first argument. An implicit root would hide the scope of termination. |
| `signal(handle, signal)` | None | Explicit target. Works outside. Non-kill signals need a live target. | A self form would save one argument. The explicit target makes state changes visible. Self-kill on the scheduled thread can yield. |
| `signal_tree(handle, signal, options?)` | None | Explicit root. Works outside. Ownership needs a live root. Creation can address a retained root. | The root and relation determine which processes change state. Both are important to the caller. |
| `children(handle, options?)` | None | Explicit root. Works outside. Ownership needs a live root. Creation can address a retained root. | A self form could shorten frequent local queries. The explicit form makes the root clear. |
| `descendants(handle, options?)` | None | Same root validation as `children`. Works outside. | The same self shorthand could apply. The option still selects which relation to traverse. |
| `get_priority(handle)` | None | Live handle. Works outside. | A self form could help code that often reads its own priority. The query is read-only. |
| `set_priority(handle, priority)` | None | Live handle. Works outside. | Keep the target visible. The assignment can change the next tick's schedule. |
| `status(handle)` | None | Explicit handle. Works outside. Returns `nil` when dead. | A self form has limited use. The explicit form supports inspecting processes that cannot currently run. |
| `alive(handle)` | None | Explicit handle. Works outside. Returns false when dead. | A scheduled-context form would usually return true. The explicit predicate also covers terminated processes. |
| Process constructors and `spawn(constructor, ...)` | Current creator or closing process, when present | No process handle. Root creation works outside. | Implicit ancestry keeps spawning short. Attachment can affect child lifetime. This path uses the runner's active or closing ID without checking caller thread identity. |
| `find(constructor)` and `snapshot()` | None | No process handle. Both work outside. | These inspect the runtime. `find` checks a process constructor. Neither needs a current process. |
| `camera_follow(target)` and `camera_follow(camera, target)` | None. The first form selects the main camera. | Live target handle. Both work outside. | Both camera choices are useful. Keep the target visible because the camera continues to track it. |
| Immediate `draw_*` commands | Scheduled process owns the commands | No process handle or outside call | The caller's drawing lifetime is the natural default. The binding checks thread identity and liveness. |
| `write` and managed asset creation | The active process can own the resource | No process handle. Supported operations can create runtime-owned resources outside. | Automatic ownership ties cleanup to the process. These paths use the active ID and may attribute a nested coroutine's resource to the scheduled caller. |

The contextual forms of collision, world movement, graph selection, animation
selection, reception, and elapsed playback all check that the calling Lua
thread is the scheduled coroutine and that the process is live. The flat and
namespace names above are aliases where the inventories list both names.

## Lua entry forms

Both `mv.start(init, options)` and a returned game table with `init`, `width`, `height`, `title`, and `fps` use the same runtime and scheduler. Initialization runs once before the first scheduled tick. Both forms use the same defaults and report initialization or process failures. Use one startup form per run. See [Lua processes](lua-processes.md) for the full options and errors.

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
| Static world | `mv_collision_world_create`, `mv_collision_world_destroy`, `mv_collision_world_add_rect`, `mv_collision_world_query`, `mv_collision_world_sweep`, `mv_collision_world_move` | `world_create`, `world_add_rect`, `world_overlap`, `world_sweep`, `world_collision`, `world_move` |

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

## Process identifier comparison with BennuGD2

The pinned BennuGD2 [identifier allocator](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/core/bgdrtm/instance.c#L248) searches for a free identifier and wraps to the start of its configured range when needed. An identifier can therefore be reused after an instance ends. This observation is limited to that allocator.

Malvado issues increasing IDs within each context and does not reuse them. A C process reference also carries runtime identity; a retained handle preserves the ended process identity and reason. The C `c_process_handle` test checks repeated creation after termination and rejects references from another context. The Lua `natural_cascade` test checks a later ID and the earlier dead handle. See the [C contract](c-contract.md) and [Lua guide](lua-processes.md).

The [C message example](../../examples/c/messages/README.md) demonstrates the message API. `c_messages_example`, `c_process`, `c_message_ownership`, and `c_query_allocation` check delivery, queue limits, ownership, and allocation failures. Lua `messages`, `queue_wrap`, and `message_gc` check the binding. See the [C contract](c-contract.md#exchange-messages) and [Lua guide](lua-processes.md#send-and-receive-messages).

The [C resource example](../../examples/c/resources/README.md) covers creation, retention, transfer, data access, and release. The `c_resources_example`, `c_resource_ownership`, and `c_process_handle` tests check lifetimes, invalid owners, shutdown, and allocation failure. The [resource contract](resource-lifetime.md) describes Lua wrappers and their retention; the C API is defined in the [C contract](c-contract.md#owned-resources). Lua resource behavior is checked by `lua_cleanup_order`, `lua_text_ownership`, and the graphics and audio resource tests on supported backends.

The [C clock example](../../examples/c/clock/README.md) connects logical clock steps to scheduler ticks. `c_clock_example`, `c_context`, `clock`, `schedule`, and Lua `cadence` and `cadence_all` check time validation, dropped time, scheduling separation, and cadence. See the [C contract](c-contract.md#create-and-close-a-context) and [Lua guide](lua-processes.md).
