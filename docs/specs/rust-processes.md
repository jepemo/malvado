# Rust process and scheduler design

Status: the owned-future adapter is implemented and verified for A7.3; see the [proof and limits](../versions/2.0.0-alpha.7-scheduler.md). A struct can use an owned async `run(self, ProcessContext)`. Macros and visual field syntax remain outside this proof. Native process properties are authoritative: do not mirror `x`, `y`, `z`, `angle` or `size` in Rust user state. Recommend typed state plus a context-bound process handle with checked native property accessors; generated convenience is optional after safety validation. `z` remains 2D draw order.

## Execution and lifetime

Prototype a single-threaded, non-`Send` adapter registered as a native process callback. On creation, associate a stable heap allocation with a native process identity and pin its future. The native scheduler invokes the adapter once per eligible activation, in its existing creation and tick order. The first activation and children spawned during a tick must match the native scheduler, measured by existing trace tests; do not assume they run immediately.

A self-borrowing `async fn run(&mut self, &mut Context)` cannot safely be stored beside its borrowed state in an ordinary movable Rust struct. Explore an owned async future that owns state and a restricted context token, or a proven safe scoped construction; never manufacture a self-reference with unchecked lifetime extension. Do not expose mutable state through handles while the future owns it. Choose the final shape only after Miri and cancellation experiments. Typed external inspection may use immutable snapshots or explicit messages, not aliases to suspended mutable state.

`frame().await` returns Pending for the current activation and requests the next eligible native activation. A tick-count wait or FPS cadence delegates to native timing; confirm exact frame(n) and FPS contracts against alpha.6. Ready requests ordinary process completion. Native kill, sleep, freeze, wake, parent/owner relationships, kill-tree and shutdown remain authoritative. No thread per process, Tokio, or separate executor.

A local waker records readiness without polling recursively or on another thread; a scheduler-owned wake queue or an explicitly restricted frame-only future is needed. Define behavior for wake during poll, duplicate wake, after cancellation and external asynchronous I/O before allowing arbitrary futures. A no-op waker is valid only for an explicitly limited frame primitive with native rescheduling. Pending without a supported wake must not busy-poll. Reject unsupported/blocking operations in documentation; the engine cannot preempt a synchronous blocking callback.

## Cleanup, errors and safety

Native safe points decide when destruction occurs. Natural Ready, external kill, self-kill, failure and shutdown each schedule exactly one cleanup: remove native association, invalidate access, then drop the pinned future and its owned state once. Do not drop while it is being polled; self-kill defers destruction until callback return. Owned native handles preserve their documented dead identity and reason; stale or foreign-context operations return errors. Attached and detached spawning map directly to existing owner and creator contracts, never to Rust task parenting.

Wrap every Rust callback entering through C with `catch_unwind` when unwinding is enabled; convert panic to a documented process failure and clean up at the safe point. No unwind crosses C. `panic=abort` terminates the program and cannot be recovered; specify this limitation. Test destructor panics separately and avoid double panic across FFI. Shared resources use native retain/release and do not outlive their context without an explicit native guarantee.

Before expanding the SDK, demonstrate deterministic frame traces, local values across suspension, spawn timing, stop/sleep/freeze, self/external kill, shutdown, wake races, panic containment and exactly-once Drop under sanitizers and Miri where applicable. Redesign if sound lifetimes require unchecked aliases, wake handling creates a second scheduler, or safe-point cleanup cannot be guaranteed. Macro ergonomics never override safety.

The implemented ownership/wake decision and current test evidence are recorded
in the proof linked above. Earlier alternatives in this design explain the
constraints; they do not add another supported process API.
