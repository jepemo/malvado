# Rust processes

The `malvado` crate provides one owned async process API over the C scheduler.
The default crate supports headless processes; optional `graphics` and `assets`
features add the delivered native visual, input, audio and asset services. Use
[SDK and projects](../site/sdk.md#create-a-rust-game) to create an installed game.
See the [language matrix](language-parity.md) for shared behavior and differences.

Use the toolchain pinned in mise:

```sh
mise install
mise run test-rust
MALVADO_SDK_PREFIX="$PWD/build-rust/Debug/relocated sdk" mise exec -- \
  cargo run --manifest-path bindings/rust/Cargo.toml -p malvado --example counter
```

`malvado/examples/counter.rs` is a runnable SDK verification program. Its game state is
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
# let mut engine = malvado::Engine::new()?;
# engine.spawn(Default::default(), |p| Counter { value: 0 }.run(p))?;
# for _ in 0..4 { engine.tick()?; }
# assert_eq!(engine.process_count(), 0);
# Ok::<(), Error>(())
```

Create an `Engine`, then call `engine.spawn(SpawnOptions::default(), |process|
Counter { value: 0 }.run(process))`. Each `engine.tick()` executes one native
scheduler tick. It does not read wall time, render, or run another executor.
`Engine::new()` configures a 60 Hz native clock with a five-step advance limit;
`advance_time` plans elapsed-time ticks and `tick` executes them. See
[host timing](runner-timing.md#rust-host-integration) for the explicit input,
audio and drawing order. The native
scheduler decides creation order and priority. No process runs inside spawn.
Its factory runs at first activation, inside the panic boundary.

## State, tokens, and handles

A future owns its state and must be `'static`; this does not require leaking
state. Moving owned game data into the future satisfies that bound. Locals and
mutable borrows of those locals can span awaits because the future is pinned.
Do not store native visual properties in parallel Rust fields. Use the handle's native position, z, angle, scale and graph accessors.
`z` remains draw depth.

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
created during a tick first runs at the next tick. `signal(target, signal)` targets one process.
`signal_tree(target, signal, Relation::Ownership)` follows ownership;
`Relation::Creation` follows historical creation, including detached children
and descendants of an ended creator. The relation is explicit rather than a
boolean flag. `handle.children(relation, type_id)` and
`handle.descendants(relation, type_id)` return owned handles in native creation
order; type zero selects every type. `engine.find(type_id)` also includes sleeping
and frozen processes. `engine.lineage()` copies retained creation nodes. Dead
ancestors remain only while live descendants need them. Ownership queries on
a dead owner fail; creation queries can still return its live descendants.

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
limits. Dedicated graphics and audio wrappers define their platform lifetimes.

## Native resource ownership

`engine.resource(value)` creates a context-owned resource.
`process.resource(value)` creates a resource owned by that live process.
`Resource<T>` owns one independent retained reference and exposes shared access
through `Deref`; it does not provide `DerefMut`. Values must own their data and
be `'static`. Use explicit safe interior mutability when shared updates are
needed. Native ownership and the retained reference have separate lifetimes.

Dropping a wrapper releases its retained reference. The value drops once after
both the owner and all retained references release it. A retained reference can
outlive the process or engine; `try_clone()` and shared reads still work then.
`resource.transfer_to(Some(&handle))` changes the native owner; None promotes it
to the same context. Foreign, dead or closed owners fail without changing the
previous ownership. A retained resource whose owner ended can acquire a new
live owner while its context exists. Context shutdown forbids later transfers.

Resource creation consumes the Rust value; on a native creation error Rust drops
it rather than transferring it to C. Each successful retain needs one release;
`try_clone()` reports native overflow. The wrappers stay on their owner thread.
Destructor panics are contained at the native resource callback. The resource
manager retains only the value it owns; it does not infer platform dependencies.
Owned `Image` and `Font` values retain their window; `Sound` and `Music` retain
their audio service. They can be placed in Resource wrappers and keep those
services live until final destruction. Native raw-pointer adoption does not
retain a device. See [managed platform resources](managed-platform-resources.md)
and the [window guide](rust-window.md).
