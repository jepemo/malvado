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

For context, three earlier runs of the same script with the pre-change debug runtime and headless execution took 101.660, 102.150, and 101.470 ms to complete. The earlier runtime included the graphics backend, but ran with `--headless`; the later build omitted it. This is a local comparison, not a guarantee for other workloads or machines. The scheduler still performs lookups that grow with process count. Rendering, memory, and a controlled comparison with the historical engine remain to be measured.
