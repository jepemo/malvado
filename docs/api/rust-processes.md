# Rust processes

Rust support is experimental in Malvado 2.0. The gameplay SDK is functional,
maintained and tested over the shared native C scheduler. Its public API can
change in future 2.x releases before Rust support is declared stable. Significant
changes include migration guidance; no long-term API stability is promised.
See the [support policy](rust-support.md) for platform and toolchain limits.

The `malvado` crate provides one owned async process API over the C scheduler.
The default crate supports headless processes; optional `graphics` and `assets`
features add the delivered native visual, input, audio and asset services. Use
[SDK and projects](../site/sdk.md#create-a-rust-game) to create an installed game.
See the [language matrix](language-parity.md) for shared behavior and differences.

## Start a game

Use an ordinary named async function for each process. Give each process its
own local state and let it yield with `frame().await`. The following bounded
example opens a window and displays a greeting for 120 process activations.

```rust
use malvado::prelude::*;

async fn hello(p: ProcessContext) -> Result<(), Error> {
    for _ in 0..120 {
        p.draw_text("Hello, Malvado!", (40, 40), 48, WHITE)?;
        p.frame().await;
    }
    Ok(())
}

let game = Game::windowed(800, 600, "Hello world")?;
game.spawn(hello)?;
game.run()?;
# Ok::<(), Error>(())
```

The `graphics` feature supplies the window and input. The game runs until
the window closes, the player presses Escape, a process calls `p.quit()`, or all
processes end. See [managed and manual windows](rust-window.md) for service
ownership and main-thread requirements.

Pass arguments through a closure such as `game.spawn(move |p| enemy(p, x, y))`.
Use `game.spawn_with(options, worker)` to specify non-default priority,
cadence, or a native type ID. The scheduler decides activation order using
priority and creation order. The factory runs at first activation inside
the panic boundary.

See the [Rust lessons](../../examples/rust/README.md) for runnable Hello World and
moving balls. The [Catch! lesson](../../examples/catch/README.md) shows collision
and scoring with two processes.

### Advanced headless or manual host

`Game::new()` creates a headless-capable context. Drive it directly with
`tick()` for deterministic tests. See [runner timing](runner-timing.md#rust-host-integration)
for elapsed-time planning and the explicit input, audio, and drawing order.
The [manual window guide](rust-window.md#manual-host) shows a bounded render loop.

```rust
use malvado::prelude::*;

async fn worker(p: ProcessContext) -> Result<(), Error> {
    p.frame().await;
    Ok(())
}

let mut game = Game::new()?;
game.spawn(worker)?;
while game.process_count() != 0 {
    game.tick()?;
}
# Ok::<(), Error>(())
```

## State, tokens, and handles

A future owns its state and must be `'static`; this does not require leaking
state. Moving owned game data into the future satisfies that bound. Locals and
mutable borrows of those locals can span awaits because the future is pinned.
Do not store native visual properties in parallel Rust fields. Use `p.position()`, `p.set_position(x, y)`, the individual `x/y/z`
accessors, and the native `angle` and `scale` methods. Setting `x` or `y`
preserves the other coordinates. `z` stays the draw depth.

`ProcessContext` is an owned, cloneable context token. It can create children,
request a frame wait, signal same-context handles, and return the current
`Handle`. Use `p.set_graphic(rect(80.0, 16.0).color(CYAN))` for a
reusable primitive, `p.collides_with(&other)` for a checked collision, and
`p.draw_text(...)`, `p.draw_box(...)`, or `p.draw_circle(...)` for commands.
Commands clear before the next actual activation. Native handle methods remain
available for operations on another process. It cannot tick or destroy the game. `Game`, tokens and handles are
neither Send nor Sync. Compile-fail doctests verify thread and borrow rules.
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

`SpawnOptions` controls native priority, FPS, type and attachment. Defaults
are priority/type zero, inherited FPS, and attached children. Root spawns have no
creator or owner. `game.set_fps(fps)` sets the default for future root and child
spawns in that game. Its initial value is zero. `None` inherits the default at
spawn time. `Some(fps)` overrides it. Valid values are 0 through 60. Both 0
and 60 mean every eligible tick. Invalid FPS fails before process creation.

`p.spawn(worker)` creates an attached child. Use
`p.spawn_with(SpawnOptions { attached: false, ..Default::default() }, worker)`
to create a detached child. Both record the current process as creator.
An attached child ends with its owner. A child spawned during a tick first
runs on a subsequent tick. `signal(target, signal)` targets one process.
`signal_tree(target, signal, Relation::Ownership)` follows ownership.
`Relation::Creation` follows historical creation, including detached children
and descendants of an ended creator. The relation is explicit rather than a
boolean flag. `handle.children(relation, type_id)` and
`handle.descendants(relation, type_id)` return owned handles in native creation
order; type zero selects every type. `game.find(type_id)` also includes sleeping
and frozen processes. `game.lineage()` copies retained creation nodes. Dead
ancestors remain only while live descendants need them. Ownership queries on
a dead owner fail; creation queries can still return its live descendants.

`process.frame().await` waits for the next eligible activation. Native Sleep and
Freeze suspend execution; Wake resumes it with locals intact. Wakes only mark
readiness for the next eligible native callback. Wakers can cross threads.
The game, process tokens and state stay on their owner thread.
`p.delta()` reports simulation seconds since the previous actual poll. Its
first value is zero. It includes logical ticks spent waiting for cadence, waking
from sleep/freeze, or waiting for a Rust waker. See [Rust process timing](runner-timing.md#rust-process-timing). A future
that returns Pending without arranging a wake stays pending. A late wake
cannot revive a cancelled process. Code must yield control to the game loop.
Malvado supplies frame waits and leaves asynchronous I/O to the host.

Returning `Ok(())` requests native completion. Returning `Err` or panicking
requests a native process error, which `game.tick()` reports as
`Error::Process`. That aggregate error does not preserve the
individual async error payload. Killing a process cancels its future at the
native safe point after poll. Dropping the engine invalidates tokens and cancels
all remaining processes. Handles do not keep process state alive.

Unwinding panics in poll, factory, and native cleanup are contained. A destructor
panic preserves the existing native end reason. An exceptional secondary panic
payload may be retained to avoid unwinding across C. `panic=abort`, double-panic
aborts and aborting panic hooks still terminate the program.

## Native resource ownership

`game.resource(value)` creates a context-owned resource.
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
