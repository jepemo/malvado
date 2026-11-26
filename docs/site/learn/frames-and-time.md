# Frames and time

A bullet should travel at its chosen speed even if its process takes fewer
turns. Measure how much simulation time has passed since its last turn,
then use that time to decide how far to move.

## Move at two activation rates

Install the [desktop SDK](../../start/installation/), download `movement.lua`,
and run it from the download directory. Both processes move at 120 logical
pixels per second and stop after 0.10 logical seconds. The example needs no
window or assets.

```sh
malvado run --headless movement.lua
```

<!-- example -->

```text
60 fps: 12.00 px after 0.10 s
30 fps: 12.00 px after 0.10 s
```

## Count time between turns

The Lua runner advances the simulation in ticks of 1/60 second. A process's
`fps` controls its activation cadence. At 30 fps it normally takes a turn
every two eligible ticks after its first turn. Values of 0 and 60 run on
every eligible tick. `mv.frame()` waits for the next activation of the
calling process. `mv.set_fps` changes the default for future Lua spawns.

On the first activation, `self.delta` is zero. After that, it reports the
logical seconds since the previous activation. The 30-fps mover takes fewer
turns, but each step covers more distance. Multiplying speed by `self.delta`
gives both movers the same distance over the same elapsed simulation time.

The renderer has its own schedule. A rendered frame can contain zero
simulation ticks or up to five when the Lua runner catches up. Rendering can
show the same simulation state more than once. Headless mode advances ticks
without pacing them to a wall clock. Use `mv.time_seconds()` to measure elapsed
wall time by subtracting two readings.

After sleep or freeze, the next delta includes ticks spent suspended. If a
character should stay still during that time, skip movement on the first turn
after waking. Applying the full delta can otherwise cause a jump.

## Try a change

Change the second mover's `fps` from 30 to 20. It takes a turn every three
eligible ticks after its first turn. Run it again and check that it still
reaches 12 pixels. For comparison, change the movement line to add
`speed / 60` every turn for both movers and compare their distances again.

## Learn more

See [Lua process timing](https://github.com/jepemo/malvado/blob/main/docs/api/lua-processes.md#process-timing)
for cadence and clock details. In experimental Rust, use
[`p.frame().await` and `p.delta()`](https://github.com/jepemo/malvado/blob/main/docs/api/rust-processes.md#activation-and-cancellation).
A [C host](https://github.com/jepemo/malvado/blob/main/docs/api/c-contract.md#create-and-close-a-context)
drives its own ticks and can use the native cadence helper. Continue with
[process communication](process-communication.md).
