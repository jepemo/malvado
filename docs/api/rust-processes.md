# Rust processes

The `malvado` crate provides one owned async process API over the C scheduler.
The current API supports headless processes. Graphics and the complete native
resource wrapper family remain in alpha.7 A7.6. This is not a full game runner.

Use the toolchain pinned in mise:

```sh
mise install
mise run test-rust
MALVADO_SDK_PREFIX="$PWD/build-rust/Debug/relocated sdk" mise exec -- \
  cargo run --manifest-path bindings/rust/Cargo.toml -p malvado --example counter
```

`malvado/examples/counter.rs` is a complete runnable example. Its game state is
an ordinary Rust struct:

```rust
use malvado::{Error, ProcessContext};

struct Counter { value: u32 }
impl Counter {
    async fn run(mut self, process: ProcessContext) -> Result<(), Error> {
        for _ in 0..3 {
            self.value += 1;
            process.frame().await;
        }
        Ok(())
    }
}
```

Create an `Engine`, then call `engine.spawn(SpawnOptions::default(), |process|
Counter { value: 0 }.run(process))`. Each `engine.tick()` executes one native
scheduler tick. It does not read wall time, render, or run another executor.
`Engine::new()` configures a 60 Hz native clock with a five-step advance limit;
the current safe API drives explicit ticks, not clock advances. The native
scheduler decides creation order and priority. No process runs inside spawn.
Its factory runs at first activation, inside the panic boundary.

## State, tokens, and handles

A future owns its state and must be `'static`; this does not require leaking
state. Moving owned game data into the future satisfies that bound. Locals and
mutable borrows of those locals can span awaits because the future is pinned.
Do not store native visual properties in parallel Rust fields. Their native
accessors are A7.6 work. `z` remains draw depth.

`ProcessContext` is an owned, cloneable context token. It can create children,
request a frame wait, signal same-context handles, and return the current
`Handle`. It cannot tick or destroy the engine. Neither engines, tokens, nor
handles are Send or Sync. Compile-fail doctests verify thread and borrow rules.
No macros or second process definition style are needed.

Spawn returns a cloneable owned handle. `process.current()` has the same native
identity as that handle. Equality compares full native runtime/process identity,
so equal local numeric IDs from different engines are not equal handles. `id()`
is informational and context-local; it cannot be used to bypass handle checks.

`handle.snapshot()` returns copied native liveness, status or end reason, type,
priority, creator and owner IDs. Dead native handles preserve only identity and
reason; the other snapshot values are zero or absent. It never exposes Rust
state or mutable storage. `set_priority` and `set_type` change the native record.
Priorities are snapshotted at tick start. Changes affect the next tick's order.

A dead handle stays inspectable after engine shutdown. State mutation returns
`Error::Closed` after shutdown and a native not-found error for an ended process
in a live context. Foreign signal targets return `Error::ForeignContext` before
any local ID operation. `kill()` returns whether it newly killed a process;
repeated kill of an ended process in a live context returns false.

## Activation and cancellation

`SpawnOptions` controls native priority, FPS, type and attachment. Defaults are
priority/type/FPS zero and attached children. Root spawns have no creator or
owner. FPS must be 0–60; 0 and 60 run at every eligible tick. Invalid FPS fails
before creating a process. Spawn errors do not transfer the caller's factory.

`process.spawn` records the current process as creator. Attached children have
that process as owner and end with it; detached children have no owner. A child
created during a tick first runs at the next tick. `signal(target, signal, tree)`
uses the native ownership tree when tree is true. Creation-tree queries and the
complete lifecycle/resource stress matrix remain A7.5 work.

`process.frame().await` waits for the next eligible activation. Native Sleep and
Freeze suspend execution; Wake resumes it with locals intact. Wakes only mark
readiness. A future that returns Pending without arranging a wake stays pending
without busy polling. Waking during poll or from another thread does not poll
recursively; it takes effect at an eligible native callback. Arbitrary futures
must obey Rust's wake contract and must not block. No asynchronous I/O runtime
is installed or promised.

Returning `Ok(())` requests native completion. Returning `Err` or panicking
requests a native process error, which `engine.tick()` reports as
`Error::Process`. That aggregate error does not preserve the
individual async error payload. Killing a process cancels its future at the
native safe point after poll. Dropping the engine invalidates tokens and cancels
all remaining processes. Handles do not keep process state alive.

Unwinding panics in poll, factory, and native cleanup are contained. A destructor
panic preserves the existing native end reason. An exceptional secondary panic
payload may be retained to avoid unwinding across C. `panic=abort`, double-panic
aborts and aborting panic hooks cannot be caught. See the
[scheduler proof](../versions/2.0.0-alpha.7-scheduler.md) for safety evidence and
limits. Native graphics/audio resource lifetime is not yet wrapped by this API.
