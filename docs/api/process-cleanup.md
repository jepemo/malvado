# Process cleanup contract

This is the alpha.5 target for C and Lua cleanup. The order test covers the basic Lua ownership cascade. The remaining implementation and error cases stay open in the [alpha.5 checklist](../versions/2.0.0-alpha.5.md).

## Termination and order

A return, error, or kill marks the process dead. A kill also marks its attached ownership descendants dead before cleanup begins. A dead process is absent from live queries. A Lua handle exposes only its identity, dead status, and reason; other property access fails. The reason does not change during cleanup.

Cleanup runs at a scheduler safe point after the active callback returns. For the set of processes marked dead, use creation order. An owner is created before its attached child, so its cleanup runs first. If a cleanup callback ends another process, add it to the pending work and use the same order among remaining dead processes. Do not call a process destructor twice. Release queued messages before the process destructor, release its owned resource references after that destructor, and then prune unneeded creation history. A resource with another retained reference stays alive.

A Lua coroutine closes its pending variables during its destructor. An error in one close callback is reported and marks the run as failed. Continue cleanup of other processes and resources. Do not resume the killed coroutine. A C callback that kills itself must return promptly; the runtime cannot interrupt its C stack.

## Reentrancy and shutdown

A cleanup callback cannot run another scheduler tick or destroy the active context. It can inspect live state and signal other live processes. It can create only an explicitly independent process. Such a process keeps its creator relationship and can run on a later tick; it is not part of the original termination snapshot. Reject an attached spawn before creating a process. These rules apply to both C and Lua; the remaining C cases must be implemented and tested before the ownership check closes.

At context shutdown, mark all processes dead before destructors run. Reject every spawn, including independent spawns. Release process and game owner references, then discard the context's lineage. Explicit resource references retained outside the context remain valid until released. A cleanup callback must not recursively destroy the context. The runtime continues shutdown after an error and reports failure through the runner.
