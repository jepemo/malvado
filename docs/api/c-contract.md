# C process API

Want to run processes without Lua? Start with the [C process example](../../examples/c/processes/README.md). For a trace that you can compare with Lua, try the [C and Lua parity example](../../examples/c/parity/README.md).

This page describes the experimental alpha.2 API in [`malvado.h`](../../src/include/malvado/malvado.h). It is a source-level contract. ABI stability is not guaranteed yet. Use a context and its processes on one owner thread; the API does not synchronize concurrent calls.

## Create and close a context

`mv_context_create(ticks_per_second, max_steps)` creates a context. Both arguments must be positive. The tick rate must be finite. A failure returns `NULL`, including when the operating system random source is unavailable. Call `mv_context_destroy(ctx)` when finished; it accepts `NULL`. The context owns the processes that were successfully added to it.

Do not destroy a context from a process callback or destructor. Such a call is ignored. Destroy it after the tick returns. During context shutdown, adding processes, sending or receiving messages, and sending signals return `MV_BUSY`.

`mv_context_advance(ctx, elapsed_seconds, &steps)` advances the logical clock. It does **not** run process callbacks. Elapsed time must be finite and nonnegative. The call reports the number of steps, limited by `max_steps`. Excess time is recorded by `mv_context_dropped_seconds`. `mv_context_ticks` returns the total clock steps. On `MV_INVALID_ARGUMENT` or `MV_OVERFLOW`, the context and output are unchanged.

For a 60-tick-per-second scheduler, initialize `MvCadence cadence = {0}`. Run a process's first activation, then call `mv_cadence_step(&cadence, fps)` on each eligible tick. It returns 1 when the process is due, 0 when it should wait, or -1 for an invalid argument or state. Rates 1 through 59 distribute activations across ticks without accumulated drift. Rates 0 and 60 run on every tick and reset the phase. Do not call it while the process is sleeping or frozen. The Lua `self.fps` field uses the same cadence.

To run processes, call `mv_process_tick` once for each logical step you want to execute. This separation lets you test time and scheduling independently. A recursive tick returns `MV_BUSY`.

## Processes and signals

`mv_process_ref_create` makes a copyable context-scoped reference from a process ID. `mv_process_ref_resolve` validates it against a live context and returns its ID, even after the process ends. A reference from another context is rejected without changing the output. Each runtime uses 128 random bits from the operating system and a process-local sequence. This makes accidental identity collisions across application restarts extremely unlikely. `mv_context_runtime_id` copies the runtime identity for an inspector. Do not modify reference fields. Existing ID-based APIs remain available during migration.

`mv_process_spawn` accepts a parent ID, priority, step callback, optional destructor, user pointer, and output ID. Parent `0` creates a root. A nonzero parent must be live. The process retains the user pointer until removal. If a destructor is provided, a successful spawn transfers responsibility for the pointer to that destructor; it is called once on removal or context shutdown. Without a destructor, the caller must keep the pointer valid for the process lifetime. When spawn fails, responsibility stays with the caller and the output ID is unchanged. IDs are positive, increase monotonically, and are not reused in the context.

The experimental `mv_process_spawn_related` accepts separate creator and owner IDs. Both must be live in the context or zero. The snapshot records the creator ID even after that creator ends; `parent` reports the current owner. This is only the first step toward the planned model: it ends owned children when their owner ends and retains creation ancestry while descendants are alive. `mv_process_spawn` sets both IDs to its parent argument and links the child to its owner. Use `mv_process_spawn_related` with owner zero to create an independent child.

The scheduler calls a runnable process once per tick. Return `MV_STEP_YIELD` to run again on a later tick, `MV_STEP_DONE` to finish, or `MV_STEP_ERROR` to report an error. A C callback returns to the scheduler; its C stack is not suspended. Keep persistent state in the user pointer. A callback error returns `MV_PROCESS_ERROR` from the tick after deferred cleanup. Processes added during a tick start no earlier than the next tick.

Higher priorities run first. Ties use creation order. The scheduler fixes the order at the start of each tick. A priority change during a tick changes the next tick's order. `mv_process_set_priority` changes priority; `mv_process_priority` reads it. `mv_process_set_type` sets a caller-defined type ID; `0` means no type.

`mv_process_signal(ctx, id, signal, tree)` accepts `MV_SIGNAL_SLEEP`, `MV_SIGNAL_FREEZE`, `MV_SIGNAL_WAKEUP`, and `MV_SIGNAL_KILL`. If `tree` is nonzero, the signal also applies to the live descendants at the time of the call. Later children do not inherit it. Sleeping and frozen processes remain alive but do not run. The Lua rendering layer hides sleeping processes and keeps the last drawing of frozen processes. A killed process is no longer available through public lookups. Cleanup is deferred until the current callback returns. A process that kills itself continues until it returns.

`mv_process_kill(ctx, ref, &killed)` validates a context-scoped reference and ends the process and its attached descendants. It sets `killed` to 1 if the process was alive, or 0 if it was already dead. A foreign or invalid reference is rejected without changing `killed`. A C callback that kills itself must return promptly; C code is not interrupted. The scheduler does not run that process again.

During process cleanup, a C destructor can create an independent process with owner zero. It can use the process being cleaned as the creator. A spawn with an owner returns `MV_BUSY` without changing the output ID. During context shutdown, all spawns return `MV_BUSY`. See the [cleanup contract](process-cleanup.md) for order and reentrancy.

Tree signals capture their live recipients before they change any state. A failure to allocate the snapshot returns `MV_OUT_OF_MEMORY` without changing any process. Processes created later do not receive that signal.

An unknown or killed ID returns `MV_NOT_FOUND`. Invalid arguments or signals return `MV_INVALID_ARGUMENT`. An invalid signal or ID does not change the tree. `mv_process_status` reads a live process state; `mv_process_count` returns the number of live processes. A suspended process counts as live.

## Inspect a snapshot

`mv_process_snapshot` copies live process records in creation order. Each record contains ID, current parent, creator ID, attachment flag, type, priority, state, and queued-message count. Pass `NULL` for `out` only when capacity is zero. `total` receives the complete number even if the buffer is smaller. The caller owns the copied buffer. Changing a copy does not change the scheduler. An owned child ends with its owner. Independent children have no owner.

`mv_process_inspection_snapshot` copies live processes and retained creation nodes in one capture. If either buffer is too small, it returns `MV_BUFFER_TOO_SMALL`, writes both required counts, and leaves both buffers unchanged. Pass null buffers with zero capacity to get the required counts. No callbacks run during capture. The caller owns both buffers.

`mv_process_children` and `mv_process_descendants` copy live processes under a live owner in creation order. Pass type `0` to include all types, or a nonzero type ID to filter. The buffer and `total` rules match `mv_process_snapshot`. The current functions use ownership only. `mv_process_creation_descendants` and `mv_process_signal_creation_tree` use creation ancestry and accept a dead ancestor ID from the same context. An ancestor with no remaining descendants gives an empty result. The scheduler prunes history that no live process needs. `mv_process_lineage_snapshot` copies the retained nodes in creation order, with each node’s creator and live state.

All snapshot records are values in caller-owned storage. They do not keep processes or lineage nodes alive. A record can become stale after the call; validate an operation against the context again before using it. The context must remain alive during a call. Use a context from one thread at a time. Do not call these functions concurrently with mutation or destruction of that context.

The single-buffer snapshot functions do not allocate. They copy at most `capacity` records and return the full count in `total`, even if the buffer is too small. A zero-capacity call can obtain the count. The caller must allow for changes before a later copy. Invalid arguments and unknown ownership roots return an error before writing the buffer or count. A creation root is an ID issued by the same context. A retained dead root can have live descendants; after its lineage is pruned, it has none. An ID greater than the last issued ID is not found. An ID alone does not prove context identity; use a context-scoped reference when identity must be validated.

No user or Lua callback runs during snapshot traversal. Captures do not change process state. The combined inspection capture checks both capacities before copying either buffer. The individual captures can copy a prefix. Tree signals allocate a recipient list before mutation; overflow or allocation failure leaves process state unchanged. The caller must not assume that separate captures describe the same instant if it runs other operations between them.

The current implementation scans the live process array and follows owner or creator links. It keeps creator nodes only while a live descendant needs them and prunes them iteratively. It has no type index. The 512-level repeated ownership query and 5,000-round creation and resource workload are recorded in the [benchmark guide](../../tools/benchmarks/README.md). These checks support retaining the current structure for alpha.5, but they do not establish a latency target for wide trees or frequent type filters. Measure those workloads before adding an index.

## Owned resources

`mv_resource_create(ctx, owner, data, destroy, &resource)` transfers `data` to the context after a successful call. The owner is a pointer to a live process reference from the same context, or `NULL` for the game context. A dead or foreign owner is rejected without changing the output. The returned pointer is borrowed. The owner holds one reference until process cleanup or context shutdown.

Call `mv_resource_retain` before sharing a resource or keeping it for pending work. Use `mv_resource_transfer(resource, &owner_ref)` to move the owner reference to a live process in the same context, or pass `NULL` to move it to the game context. The transfer does not change retained references. It can assign a new owner to a retained resource after its former owner ends. A foreign or dead owner is rejected without changing ownership. Each successful retain needs one `mv_resource_release`. When the owner and all retained references are gone, Malvado calls `destroy` once. `mv_resource_data` returns the stored pointer while a reference is valid. On a failed create, the caller keeps `data` and the output is unchanged.

Retained references can outlive the context. Release them before closing any graphics or audio service that their destructor needs. Use these operations and destructors on the context owner thread. Do not use a borrowed pointer after its owner ends unless you retained it. The Lua image, atlas, font, sound and music wrappers use this manager. Each wrapper holds a retained reference until Lua collects it. A drawing command or text object keeps its wrapper alive. Lua playback retains sound and music wrappers while they play or are paused. Completed playback is checked on scheduler ticks. Explicit transfer rules and final resource-release timing still need work. Native platform resources created directly through the C platform API are not enrolled automatically.

## Exchange messages

Each process has a FIFO queue with room for 1,024 messages. `mv_process_send` returns `MV_QUEUE_FULL` if it is full. `mv_process_receive` returns `MV_QUEUE_EMPTY` if it is empty; it does not block. Unknown or killed recipients return `MV_NOT_FOUND`. A receive failure leaves the output unchanged. Pending messages are released when the recipient ends.

Boolean, integer, and number values are copied. `MV_MESSAGE_BYTES` is copied on send, including embedded zero bytes. The input byte pointer stays with the sender. For `MV_MESSAGE_OPAQUE`, a successful send transfers the opaque pointer and its optional destructor to the queue. On failure, it stays with the sender. Use opaque values only when the recipient knows their meaning.

A successful receive transfers the returned message to the receiver. Call `mv_message_release` exactly once after using it. It frees received byte storage or calls the opaque destructor, then clears the message. Do **not** release a borrowed input byte pointer with this function. For example:

```c
MvMessage out = {.type = MV_MESSAGE_BYTES, .value.bytes = {(void *)"ping", 4}};
MvResult result = mv_process_send(ctx, recipient, &out);
/* The input string remains owned by the sender. */

MvMessage in;
if (mv_process_receive(ctx, recipient, &in) == MV_OK) {
    /* Read in.value.bytes.data and in.value.bytes.size here. */
    mv_message_release(&in);
}
```

C and Lua can exchange booleans, integers, numbers, and binary strings. The Lua binding keeps tables and other non-primitive Lua values in the Lua VM and only sends them to Lua processes. It preserves their shared identity. A Lua process rejects an opaque message from an unrelated native producer and releases it.

## Platform and embedding limits

[`platform.h`](../../src/include/malvado/platform.h) provides the initial window adapter. The raylib implementation owns one window and must run on the main thread. `mv_window_escape_down` and `mv_window_space_down` report held keys. The headless implementation cannot open a window and reports both keys as released. These functions are separate from the process context; broader graphics and resource APIs are planned for alpha.3.

The current `malvado-run` owns its Lua VM and context. Only one runner can be active at a time; a nested `malvado_run` returns exit code 2. `malvado_debug_context` is an experimental runner-thread diagnostic hook, used by the [mixed C/Lua test](../../tests/mixed_host.c). It is not a stable API for attaching a binding to an arbitrary host VM. IPC, a GUI, and a LuaRocks module for an external interpreter are not part of alpha.2.
