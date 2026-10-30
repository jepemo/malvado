# C process API

Want to run processes without Lua? Start with the [C process example](../../examples/c/processes/README.md). For a trace that you can compare with Lua, try the [C and Lua parity example](../../examples/c/parity/README.md).

This page describes the experimental alpha.2 API in [`malvado.h`](../../include/malvado/malvado.h). It is a source-level contract. ABI stability is not guaranteed yet. Use a context and its processes on one owner thread; the API does not synchronize concurrent calls.

## Create and close a context

`mv_context_create(ticks_per_second, max_steps)` creates a context. Both arguments must be positive. The tick rate must be finite. A failure returns `NULL`. Call `mv_context_destroy(ctx)` when finished; it accepts `NULL`. The context owns the processes that were successfully added to it.

Do not destroy a context from a process callback or destructor. Such a call is ignored. Destroy it after the tick returns. During context shutdown, adding processes, sending or receiving messages, and sending signals return `MV_BUSY`.

`mv_context_advance(ctx, elapsed_seconds, &steps)` advances the logical clock. It does **not** run process callbacks. Elapsed time must be finite and nonnegative. The call reports the number of steps, limited by `max_steps`. Excess time is recorded by `mv_context_dropped_seconds`. `mv_context_ticks` returns the total clock steps. On `MV_INVALID_ARGUMENT` or `MV_OVERFLOW`, the context and output are unchanged.

For a 60-tick-per-second scheduler, initialize `MvCadence cadence = {0}`. Run a process's first activation, then call `mv_cadence_step(&cadence, fps)` on each eligible tick. It returns 1 when the process is due, 0 when it should wait, or -1 for an invalid argument or state. Rates 1 through 59 distribute activations across ticks without accumulated drift. Rates 0 and 60 run on every tick and reset the phase. Do not call it while the process is sleeping or frozen. The Lua `self.fps` field uses the same cadence.

To run processes, call `mv_process_tick` once for each logical step you want to execute. This separation lets you test time and scheduling independently. A recursive tick returns `MV_BUSY`.

## Processes and signals

`mv_process_spawn` accepts a parent ID, priority, step callback, optional destructor, user pointer, and output ID. Parent `0` creates a root. A nonzero parent must be live. The process retains the user pointer until removal. If a destructor is provided, a successful spawn transfers responsibility for the pointer to that destructor; it is called once on removal or context shutdown. Without a destructor, the caller must keep the pointer valid for the process lifetime. When spawn fails, responsibility stays with the caller and the output ID is unchanged. IDs are positive, increase monotonically, and are not reused in the context.

The scheduler calls a runnable process once per tick. Return `MV_STEP_YIELD` to run again on a later tick, `MV_STEP_DONE` to finish, or `MV_STEP_ERROR` to report an error. A C callback returns to the scheduler; its C stack is not suspended. Keep persistent state in the user pointer. A callback error returns `MV_PROCESS_ERROR` from the tick after deferred cleanup. Processes added during a tick start no earlier than the next tick.

Higher priorities run first. Ties use creation order. The scheduler fixes the order at the start of each tick. A priority change during a tick changes the next tick's order. `mv_process_set_priority` changes priority; `mv_process_priority` reads it. `mv_process_set_type` sets a caller-defined type ID; `0` means no type.

`mv_process_signal(ctx, id, signal, tree)` accepts `MV_SIGNAL_SLEEP`, `MV_SIGNAL_FREEZE`, `MV_SIGNAL_WAKEUP`, and `MV_SIGNAL_KILL`. If `tree` is nonzero, the signal also applies to the live descendants at the time of the call. Later children do not inherit it. Sleeping and frozen processes remain alive but do not run. The Lua rendering layer hides sleeping processes and keeps the last drawing of frozen processes. A killed process is no longer available through public lookups. Cleanup is deferred until the current callback returns. A process that kills itself continues until it returns.

An unknown or killed ID returns `MV_NOT_FOUND`. Invalid arguments or signals return `MV_INVALID_ARGUMENT`. An invalid signal or ID does not change the tree. `mv_process_status` reads a live process state; `mv_process_count` returns the number of live processes. A suspended process counts as live.

## Inspect a snapshot

`mv_process_snapshot` copies live process records in creation order. Each record contains ID, current parent, type, priority, state, and queued-message count. Pass `NULL` for `out` only when capacity is zero. `total` receives the complete number even if the buffer is smaller. The caller owns the copied buffer. Changing a copy does not change the scheduler. When a parent ends, its surviving children become roots after cleanup.

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

[`platform.h`](../../include/malvado/platform.h) provides the initial window adapter. The raylib implementation owns one window and must run on the main thread. `mv_window_escape_down` and `mv_window_space_down` report held keys. The headless implementation cannot open a window and reports both keys as released. These functions are separate from the process context; broader graphics and resource APIs are planned for alpha.3.

The current `malvado-run` owns its Lua VM and context. Only one runner can be active at a time; a nested `malvado_run` returns exit code 2. `malvado_debug_context` is an experimental runner-thread diagnostic hook, used by the [mixed C/Lua test](../../tests/mixed_host.c). It is not a stable API for attaching a binding to an arbitrary host VM. IPC, a GUI, and a LuaRocks module for an external interpreter are not part of alpha.2.
