# Process benchmark

This benchmark creates 100 Lua processes. Each process yields 60 times. A monitor process waits for all workers to finish. It prints the time to create workers and the time from the end of creation until they finish. The run time includes scheduling, Lua resumes, worker cleanup, and the monitor. It does not measure rendering, audio, memory, or CPU time separately.

Build the headless debug runtime and run it from the repository root:

```sh
mise run build-headless
./build-lua-headless/malvado-run --headless tools/benchmarks/processes.lua
MALVADO_BENCH_PROCESSES=400 ./build-lua-headless/malvado-run --headless tools/benchmarks/processes.lua
```

The process count must be an integer from 1 to 1000. The default is 100. Run on the same machine and build type for comparisons. Close other workloads and repeat runs. These wall-clock results are not a graphics frame rate.

## Baseline

On 2026-10-30, a debug build on macOS 26.6.2, Apple M2 Pro, and Apple Clang 21.0.0 ran 100 workers with 60 yields each. Three runs after removing a full-state synchronization before every Lua resume (commit `3bb4ca9`, before later cadence changes) gave:

| Run | Creation (ms) | Completion (ms) |
| --- | ---: | ---: |
| 1 | 0.209 | 3.303 |
| 2 | 0.133 | 3.260 |
| 3 | 0.134 | 3.293 |

For context, three earlier runs of the same script with the pre-change debug runtime and headless execution took 101.660, 102.150, and 101.470 ms to complete. The earlier runtime included the graphics backend, but ran with `--headless`; the later build omitted it. This is a local comparison, not a guarantee for other workloads or machines. The scheduler still performs lookups that grow with process count. This does not measure per-process memory.

One separate run at 1000 workers reported 1.706 ms for creation and 146.428 ms for completion. On the same machine, `/usr/bin/time -l` reported 0.15 seconds of user CPU time and 5,324,800 bytes maximum resident set size for the whole runner process. This includes the Lua VM and benchmark, not just worker memory. The operating system's measurement and other workloads affect the result. Compare memory only with the same tool, system, build type, and workload.

## Rendering

The C render benchmark draws the same 50×50 PNG 1000 times per frame for 120 frames in an 800×600 window. Run it from the repository root after `mise run build`:

```sh
./build/benchmark-render
```

It reports average wall time for drawing calls and for the subsequent present and wait. The latter includes GPU submission, the display, and the 60 FPS frame limit; it is not a GPU timer. VSync is not explicitly requested. Close the window to stop early. Use the same hardware, display settings, backend, image, window size, and frame limit for comparisons.

On the same macOS machine and a debug build, three runs measured 0.113, 0.111, and 0.112 ms for drawing calls per frame, and 17.511, 16.939, and 16.967 ms for present and wait per frame. These numbers do not include Lua processes or establish performance on other devices.

## Ownership traversal

Build the C benchmark and run it from the repository root:

```sh
cmake --build build --target ownership-benchmark
./build/ownership-benchmark
```

It creates a chain of 512 owned processes and queries the root's descendants ten times. It reports process CPU time for the queries only. It excludes creation and cleanup. Use the same compiler, build type and machine for comparisons. Repeat runs; this workload does not represent every game.

The current core follows owner links directly. It also follows separate creator links and prunes unused creation nodes. Queries still scan live processes and do not have a type index. Measure a representative game before adding an index; a wide tree or frequent type queries can have different costs from this chain.

On the local macOS development build, two runs of the 512-level, ten-query workload reported 1.656 ms and 1.631 ms of process CPU time. The `c_long_session` test also completes 5,000 rounds and 20,000 processes and resources, with pruning checks. These are implementation checks, not performance limits. They do not measure wide trees or frequent type-filtered queries.

## Process collision queries

Build the headless debug runtime and run both workloads from the repository root:

```sh
mise run build-headless
./build-lua-headless/process-collision-benchmark
./build-lua-headless/malvado-run --headless tools/benchmarks/process_collision.lua
```

Each workload creates 1,000 processes. The C benchmark measures 1,000 first-hit
queries with separated circles, 1,000 collider updates, and 1,000 first-hit
queries with all circles overlapping. The Lua benchmark measures 1,000 field
updates followed by 1,000 no-hit type queries. The reported times are process
CPU time; they exclude process creation and cleanup. The Lua measurement also
includes binding and field-access costs. The C benchmark uses all process types;
the Lua benchmark selects one type. Both smoke tests verify their results but
set no performance threshold.

On 2026-11-04, three debug runs on macOS, Apple M2 Pro, reported C miss times of
127.260, 127.014, and 124.883 ms; C update times of 0.801, 0.760, and 0.899 ms;
and C dense-hit times of 0.826, 0.758, and 0.900 ms. Lua update times were
3.948, 4.041, and 3.891 ms; Lua miss times were 131.995, 121.470, and
122.690 ms. These are local measurements, not platform guarantees.

After caching transformed bounds on collider updates, three debug runs on the
same local machine reported C miss times of 5.167, 4.881, and 4.952 ms; C
update times of 0.732, 0.687, and 0.711 ms; and dense-hit times of 0.795,
0.747, and 0.723 ms. Lua update times were 3.667, 4.051, and 3.904 ms;
Lua miss times were 4.988, 4.848, and 4.809 ms. This is a local comparison,
not a performance guarantee.

The implementation still scans live processes in creation order, but uses cached
bounds to reject distant candidates before shape intersection. Updates replace
the bounds atomically with the collider; invalid updates preserve both. This
linear scan can still become expensive at larger scales or with dense scenes.
`visual_collision_render_frequency` checks identical process collision and tick
traces with one and four rendered frames per tick.
