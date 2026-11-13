# Shared runner timing

The context owns one fixed-step accumulator. It plans logical ticks without
running processes. The existing process scheduler executes those ticks. Planning
time does not read input, update audio, advance fades or draw a frame.

## C clock planning

Create a context with `mv_context_create(rate, max_steps)`. Call
`mv_context_advance(ctx, elapsed_seconds, &steps)` with a finite, nonnegative
elapsed duration. Call `mv_process_tick(ctx)` once for each planned step. Each
advance returns at most `max_steps`; excess whole-tick time is discarded and
recorded by `mv_context_dropped_seconds`. Fractional time stays in the context.

`mv_context_ticks` counts steps returned by time advances. It does not count
manual scheduler calls. A host can stop before it executes every planned step;
the clock does not put those steps back. Zero elapsed time is valid. Invalid
input or overflow leaves the plan output and all clock state unchanged.

```c
unsigned steps;
if (mv_context_advance(ctx, elapsed_seconds, &steps) == MV_OK) {
    for (unsigned i = 0; i < steps; ++i) {
        /* With a window, commit native input before each logical tick. */
        mv_input_tick();
        if (mv_process_tick(ctx) != MV_OK) break;
    }
}
```

## Lua runner integration

The Lua runner uses its context clock at 60 Hz, with at most five catch-up ticks
per rendered frame. It no longer keeps a separate private accumulator. Startup
still executes one immediate scheduler tick. Frames with no due tick still draw;
catch-up frames commit input and execute each due tick separately. Headless runs
keep their deterministic direct-tick loop.

The existing pause/step path remains explicit. Time spent paused is not added to
the accumulator. Invalid or backward platform samples contribute zero elapsed
time, as before. A native clock overflow fails the run. The runner owns clock
advancement during `start`; native extensions can inspect its counters but must
not also advance it.

The shared [timing fixture](../../tests/fixtures/runner-clock.txt) covers zero
elapsed time, fractional accumulation, a capped catch-up frame and the next
normal tick. The automated Lua display test injects those same samples into the
real loop and checks native planned-tick and dropped-time counters at each
scheduler activation. Native context tests use the same fixture.

Clock planning remains independent of window events and drawing. The
[shared rendering](runner-rendering.md) and [audio maintenance](audio-maintenance.md)
services are delivered separately; host service calls remain explicit.

## Rust host integration

`Game::advance_time(elapsed_seconds)` uses the same context planner. The default
engine runs at 60 Hz and returns at most five steps per call. `planned_ticks()`
and `dropped_seconds()` read native clock accounting. All input and overflow
errors preserve the existing clock state. Each engine has an independent clock.

```rust
use malvado::Game;
let mut game = Game::new()?;
let steps = game.advance_time(1.0 / 60.0)?;
for _ in 0..steps {
    game.tick()?;
}
assert_eq!(game.planned_ticks()?, 1);
# Ok::<(), malvado::Error>(())
```

In a graphics host, call `window.input_tick()` immediately before each
`game.tick()`. Update audio regularly and draw once per host frame. Advance
fade values once per logical tick. These services remain explicit.

Time planning does not activate a future, wake a process, or execute a tick.
Call `tick()` to consume the returned steps; do not call `advance_time` again to
consume the same plan. Direct manual ticks remain supported and do not change
the planning counter. For Lua-style immediate startup, call `tick()` once before
the elapsed-time loop.

Rust tests read the same fixture as C and the injected-time Lua runner. They
check future activation counts, independent clocks, invalid inputs, overflow
atomicity and manual-tick independence. The installed offline SDK consumer uses
all three Rust timing methods in Debug and Release.
