# Process benchmark

This benchmark creates 100 Lua processes. Each process yields 60 times. A monitor process waits for all workers to finish. It prints the time to create workers and the time from the end of creation until they finish. The run time includes scheduling, Lua resumes, worker cleanup, and the monitor. It does not measure rendering, audio, memory, or CPU time separately.

Build the headless debug runtime and run it from the repository root:

```sh
mise run build-headless
./build-lua-headless/malvado-run --headless benchmarks/processes.lua
MALVADO_BENCH_PROCESSES=400 ./build-lua-headless/malvado-run --headless benchmarks/processes.lua
```

The process count must be an integer from 1 to 1000. The default is 100. Run on the same machine and build type for comparisons. Close other workloads and repeat runs. These wall-clock results are not a graphics frame rate.

## Baseline

On 2026-10-30, a debug build on macOS 26.6.2, Apple M2 Pro, and Apple Clang 21.0.0 ran 100 workers with 60 yields each. Three runs after removing a full-state synchronization before every Lua resume gave:

| Run | Creation (ms) | Completion (ms) |
| --- | ---: | ---: |
| 1 | 0.209 | 3.303 |
| 2 | 0.133 | 3.260 |
| 3 | 0.134 | 3.293 |

For context, three earlier runs of the same script with the pre-change debug runtime and headless execution took 101.660, 102.150, and 101.470 ms to complete. The earlier runtime included the graphics backend, but ran with `--headless`; the later build omitted it. This is a local comparison, not a guarantee for other workloads or machines. The scheduler still performs lookups that grow with process count. Per-process memory and a controlled comparison with the historical engine remain to be measured.

One separate run at 1000 workers reported 1.706 ms for creation and 146.428 ms for completion. On the same machine, `/usr/bin/time -l` reported 0.15 seconds of user CPU time and 5,324,800 bytes maximum resident set size for the whole runner process. This includes the Lua VM and benchmark, not just worker memory. The operating system's measurement and other workloads affect the result. Compare memory only with the same tool, system, build type, and workload.

## Rendering

The C render benchmark draws the same 50×50 PNG 1000 times per frame for 120 frames in an 800×600 window. Run it from the repository root after `mise run build`:

```sh
./build/benchmark-render
```

It reports average wall time for drawing calls and for the subsequent present and wait. The latter includes GPU submission, the display, and the 60 FPS frame limit; it is not a GPU timer. VSync is not explicitly requested. Close the window to stop early. Use the same hardware, display settings, backend, image, window size, and frame limit for comparisons.

On the same macOS machine and a debug build, three runs measured 0.113, 0.111, and 0.112 ms for drawing calls per frame, and 17.511, 16.939, and 16.967 ms for present and wait per frame. These numbers do not include Lua processes or establish performance on other devices. A controlled comparison with the historical engine remains pending.
