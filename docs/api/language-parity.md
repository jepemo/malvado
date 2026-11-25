# C, Lua and experimental Rust support

All three APIs use the C engine and its process scheduler. Lua coroutines and
Rust owned futures adapt that scheduler; they do not run a second executor.
Rust's gameplay API is experimental, functional and maintained; this label does
not apply to the C core. See the [Rust support policy](rust-support.md).
This feature matrix covers the shared native APIs. macOS/Linux SDK and CLI paths
are verified. Windows x64 MSVC headless and graphical SDKs, CLI exports and
fresh exported players are verified. Android arm64 APK and AAB exports are
verified for C, Lua and Rust on the alpha.8 baseline. See the
[platform matrix](../development/platforms.md). This does not freeze the
2.x API or ABI.

| Capability | C | Lua | Rust (experimental) |
| --- | --- | --- | --- |
| Processes, cadence, priority, genealogy, signals and cleanup | Native callbacks and explicit context ticks | Processes/coroutines in the same context | Owned async futures, typed handles and frame waits over native callbacks |
| Clock and host loop | Context planner and explicit input/tick/draw calls | `start` owns the elapsed-time loop and immediate startup tick | `Game::run` drives a managed window; `advance_time` and `tick` are available for manual hosts |
| Native position, depth, angle, scale, graph and collider | Context-bound process operations | Fields/descriptors synchronize with the native record | Checked process context and handle methods over native state |
| PNG, atlas, Aseprite export JSON and selected FPG subset | Native loaders and explicit ownership | Shared loaders with userdata descriptors | Optional `assets`; owned metadata and borrowed views; `graphics` adds textures |
| Images, sprites, shapes, graph animation, drawing and cameras | Native storage, tick updates and explicit draw pass | Shared native pass with live descriptor preparation | Optional `graphics`; retained images/sprites, managed or explicit frame process pass |
| Persistent text and fonts | Context registry, retained style and explicit draw pass | Shared registry/pass with live font descriptor updates | Owned text/handles and retained fonts/windows; managed or explicit text pass |
| Keyboard, mouse, controller and Unicode text | Native queries and per-tick input snapshot | Same native queries and transitions | Typed queries in managed processes or on an explicit window |
| Sound and music | Explicit device/instance lifetime and shared stream update | Userdata and playback roots; runner pumps native maintenance | Optional `graphics`; owned device/instances and managed or explicit maintenance |
| Geometric collisions and static rectangle worlds | Native overlap/sweep and process movement | Contextual helpers over native selected colliders | Same native queries and movement through typed shapes/handles |
| Headless use | Core without Lua, Rust or raylib | Headless runner; graphics/audio creation unavailable | Default core crate; graphics linking requires a graphical SDK |
| Project tools and player exports | `new --lang c`, CMake `game` target and native player | `new`, syntax check and bundled runner | `new --lang rust`, Cargo `game` target and native player |

Ownership is explicit in each language. A raw C resource reference does not keep
its window/device alive. Safe Rust image/font/audio wrappers retain their required
platform owner. Lua retains descriptors and playback roots in its registry.
The shared native manager still controls process/context resource ownership.

C offers explicit host services. Lua's `start` and Rust's
`Game::windowed(...)` with `run()` provide managed gameplay loops over the same
clock, input, scheduler, and render services. Rust also supports a manual
host and direct headless ticks. Rust's generated starter uses a named async
process and the managed runner. Lua and Rust expose process delta and defaults
for newly spawned processes. C hosts choose per-process cadence explicitly.

Lua aliases, legacy table descriptors, experimental debugger hooks and Lua
`rand` are not extra Rust API styles. Use each language's standard math, random,
file and string libraries where no shared engine service exists. No native
random-stream parity or automatic mixed-language payload conversion is claimed.
C/Lua coexistence is tested internally; public dynamic C/Lua/Rust mixing and
shared user-state conversion are outside this milestone.

See [C contracts](c-contract.md), [Lua processes](lua-processes.md),
[Rust processes](rust-processes.md), [resource lifetime](resource-lifetime.md),
[runner timing](runner-timing.md), [shared rendering](runner-rendering.md),
[audio maintenance](audio-maintenance.md), and [CLI contracts](../development/cli.md).
