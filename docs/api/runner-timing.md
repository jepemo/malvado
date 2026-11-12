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

This is clock integration. Window events, rendering preparation, audio
maintenance and the remaining shared runner services are separate work.
