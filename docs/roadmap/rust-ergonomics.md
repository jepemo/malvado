# Rust API Ergonomics and DIV-Style Processes

Status: future proposal. No version is assigned. No syntax or architecture below
is approved for implementation. Reevaluate this proposal against the SDK and
real game examples when the owner schedules the work. It is not a 2.0 release
gate. The [version plan](../versions/README.md) owns the current scope.

## Current baseline and motivation

Malvado 2.0 has a functional, tested but [experimental Rust gameplay SDK](../api/rust-support.md).
`Game` is its public entry point. It owns the existing native context; it does
not create a window or run a host loop automatically. The C scheduler remains
authoritative. Rust futures own state, handles refer to canonical native process
identities, and native safe points control cancellation and destruction.

An ordinary struct with an owned `async fn run(self, ProcessContext)` already
expresses sequential behavior across frames. This remains the comparison
baseline. Preserve it for 2.0. See [Rust processes](../api/rust-processes.md) and
[the adapter design](../specs/rust-processes.md).

The goal is to make common game behavior readable with less setup. DIV's useful
idea is that a game consists of independent processes whose behavior is written
sequentially. Rust async/await can preserve local state across frame waits
without asking authors to write scheduler callbacks or state machines. Literal
DIV syntax is not the goal. Improvements must reduce real boilerplate while
remaining idiomatic, safe and maintainable.

## Candidate A: function-based declarations

Illustrative syntax only. The attributes, `Proc`, fields and constructors below
are not delivered APIs and may not be implementable as shown.

```rust
#[process]
async fn explosion(mut p: Proc) {
    p.graph = circle(10.0).color(ORANGE);
    for _ in 0..20 {
        p.size += 0.1;
        p.frame().await;
    }
}

#[process]
async fn bullet(mut p: Proc, velocity_x: f32, velocity_y: f32) {
    p.graph = circle(3.0);
    loop {
        p.x += velocity_x;
        p.y += velocity_y;
        p.frame().await;
    }
}
```

This could suit effects, bullets, animations and short behaviors. Async locals
already provide persistent typed state; a separate state struct may be needless
for small processes. An ordinary async function can already return an owned
future to the current spawn factory. Establish what a macro adds before adding
one.

Investigate process type registration, argument storage, type-specific collision
queries, native property access and the exact generated code. Any frontend must
feed the existing owned-future adapter without copying native state, scheduling
independently or changing cleanup timing.

## Candidate B: struct-based declarations

Illustrative syntax only. These attributes and implicit fields are not available.

```rust
#[derive(Process)]
struct Enemy {
    health: i32,
    speed: f32,
}

#[process]
impl Enemy {
    async fn run(&mut self) {
        self.graph = circle(20.0).color(RED);
        while self.health > 0 {
            self.x += self.speed;
            self.frame().await;
        }
    }
}
```

Structs give complex entities ordinary methods, typed state, composition and a
clear identity. Compare this candidate with today's owned `self` method. A
future borrowing `&mut self` cannot simply be stored beside the state it borrows.
Study borrowing across await points, pinning, cancellation and safe generated
adapters. Do not extend lifetimes with unchecked tricks to make the sample work.

Determine whether an owned receiver is simpler, whether either attribute is
needed, and how native fields would be accessed without mirrored state. A derive
macro cannot add fields to the original struct. Any item rewriting or property
adapter needs an explicit safety and tooling review.

## One process model, possibly two frontends

Do not discard either candidate now. Evaluate whether both can be ergonomic
frontends over one runtime process model. They must share the scheduler, process
identity, lifecycle, ownership architecture, collision code, graphics and
resource implementations. Define how handles and type registration interact
across both declarations.

Supporting both is a decision to validate, not a commitment. If their distinct
benefits do not justify maintenance, generated-code complexity and teaching
cost, recommend one preferred style and explain the trade-offs. Do not introduce
a parallel process API with different semantics.

## Other ergonomic candidates

### Reusable graphics and immediate drawing

Explore `circle(20.0)`, `rect(100.0, 100.0)` and optional configuration such as
`circle(20.0).color(BLUE)`. Adapt existing native graphic/geometry concepts;
do not implement a second renderer or geometry system. Distinguish creating a
reusable graphic from issuing an immediate drawing command. Test equivalent
colors, pivots, dimensions, transforms and ownership.

### Native properties and movement

Syntax such as `self.x += 2.0` and `self.graph = circle(20.0)` is an aspiration,
not permission to keep unsynchronized Rust copies of C process state. Investigate
what generated code can safely express. If fields are impractical, compare
compact checked accessors such as `p.set_graph(...)` or movement methods.
Readable errors and ownership matter more than literal field syntax.

Investigate process-associated world movement, including automatic association,
multiple worlds, foreign/dead handles, resource lifetime and errors. Distinguish
direct position changes from collision-aware movement before naming an operation
such as `p.move_by(dx, dy)`. Preserve existing native world algorithms. This is
not a proposal for another physics engine.

### Typed collision queries

Consider `p.collision::<Enemy>()` returning a checked process handle. Define type
registration, foreign declarations, dead targets and the result's lifetime.
Keep algorithms in C and preserve canonical runtime/process identity. A typed
query must not expose a mutable reference to state owned by a suspended future.

### Spawning and sequential waits

Compare concise forms such as `game.spawn(Enemy::new(...))`,
`p.spawn(Bullet::new(...))` and `p.spawn_detached(...)` with today's factory and
`SpawnOptions`. These are not approved signatures. Keep advanced options and
preserve parent/child, attached/detached, creator and owner contracts.

`p.frame().await` is delivered through the current process token. A helper such
as `p.wait_frames(60).await` is a candidate; define eligible activations, native
ticks, FPS changes, sleep/freeze, cancellation and boundary cases first. Do not
add helpers that merely duplicate normal Rust control flow without a benefit.

### Game initialization and lifecycle

The approved rename to `Game` does not approve automatic orchestration. Future
work can compare explicit setup with higher-level window configuration,
initialization, root spawning, a host loop, shared resources and shutdown.
Do not require a fluent builder or assume `Game::new().title(...).run()` is the
answer. Compare error paths, cleanup and control of time/input/audio/drawing
against the existing explicit services. Keep headless use possible.

## Design principles and technical risks

- Keep common cases small and readable. Prefer normal Rust functions, structs,
  methods, generics and ownership rules. Character count alone is not success.
- Preserve sequential process behavior across frames and one C-owned scheduler.
- Keep native state authoritative. No unsafe aliases, self-references or duplicate
  collision, resource or renderer implementations for attractive syntax.
- Use composition and consistent engine concepts across C, Lua and Rust. Preserve
  clear advanced APIs for cases that need explicit control.
- Do not add an ECS, independent executor or large macro framework without a
  demonstrated need. Macros must have useful diagnostics and Rust Analyzer support.
- Review expansion hygiene, type registration, generic arguments, generated code,
  panic boundaries, future size and compile-time cost. Suspension and cancellation
  must still drop owned state exactly once at the native safe point.
- Account for source migrations. Experimental status permits approved changes
  with guidance, not undocumented changes or weaker safety checks.

## Suggested evaluation sequence

1. Review real effects, bullets and complex entities written with the current SDK.
   Measure setup code and identify actual difficulties.
2. Compare ordinary functions and owned structs before prototyping macros. Keep
   both candidates open and state which problems each solves.
3. Build small isolated proofs only when this work is scheduled. Validate owned
   futures, borrowing, native properties, type registration and diagnostics.
4. Test whether both forms can use the same adapter and identities. Choose both
   or one preferred frontend with written maintenance and safety trade-offs.
5. Evaluate graphics, collision, movement, spawning and waits as separate small
   conveniences. Study higher-level `Game` initialization independently.
6. Record a design decision, bounded milestone, migration plan and acceptance
   criteria before product implementation. No future version is assigned here.

## Validation and promotion criteria

Before a design becomes release work:

- Reevaluate every sketch against the then-current SDK and real Rust game examples.
- Compare function and struct declarations, owned receivers and borrowing across
  awaits. Prove any macro adapter safe under cancellation and lifecycle operations.
- Review Rust Analyzer navigation/completion, compiler errors, expansion output,
  generated-code complexity, compile times and the boilerplate actually removed.
- Verify registration and typed queries without exposing suspended mutable state.
- Compare native property, collision, graphics, resource and genealogy behavior
  with existing C/Lua/Rust traces and installed-SDK consumers.
- Test wake behavior, frame timing, natural completion, kill/self-kill, sleep,
  freeze, shutdown, panic containment and exactly-once destruction.
- Run relevant C native sanitizers, Rust safety/Miri checks and process lifetime
  tests. Keep one scheduler and one native state/ownership architecture.
- Assess public API migration cost and provide guidance for approved breaking
  changes. A smaller API must remain useful for explicit advanced cases.

## Open decisions and deferred work

The declaration styles, attributes, receiver shape, registration, typed collision
results, property syntax, convenience signatures and `Game` lifecycle design all
remain undecided. Supporting both frontends is not promised. Stable Rust API
status requires a separate decision; this proposal does not grant it.

No macros, direct fields, alternate executor, new builder, `Game::run()`, ECS,
physics engine or process-model rewrite belongs to the Malvado 2.0 scope. Keep
using the working SDK and validate improvements progressively when scheduled.
