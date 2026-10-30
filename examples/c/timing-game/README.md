# Timing game: C and Lua

Can you stop the clock close to five seconds? Press **Space** to stop it, then press **Escape** to exit.

Build the project, then choose either version:

```sh
./build/example-c-timing-game
./build/malvado-run examples/c/timing-game/main.lua
```

Both versions use the same 60 Hz process model. The C version uses explicit state and does not link Lua. The Lua version keeps its state across `frame()` calls. For automated graphics checks, run the C program with `--smoke` or set `MALVADO_SMOKE=1` for the Lua program. These modes stop automatically.

The visual test captures the C and Lua games after the clock stops at tick 5. It compares every RGB pixel and allows a difference of at most 2 per channel. Both versions use the same display and font backend in each test run; the test does not compare output from different machines.
