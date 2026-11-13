# C, Lua and Rust support

All three APIs use the C engine and its process scheduler. Lua coroutines and
Rust owned futures adapt that scheduler; they do not run a second executor.
This matrix covers delivered capabilities on native macOS and Linux. Windows
and Android acceptance is alpha.8 work. It does not freeze the 2.x API or ABI.

| Capability | C | Lua | Rust |
| --- | --- | --- | --- |
| Processes, cadence, priority, genealogy, signals and cleanup | Native callbacks and explicit context ticks | Processes/coroutines in the same context | Owned async futures, typed handles and frame waits over native callbacks |
| Clock and host loop | Context planner and explicit input/tick/draw calls | `start` owns the elapsed-time loop and immediate startup tick | `Engine::advance_time` plans; `tick` executes; host input/audio/drawing stay explicit |
| Native position, depth, angle, scale, graph and collider | Context-bound process operations | Fields/descriptors synchronize with the native record | Checked handle methods; no mirrored Rust position state |
| PNG, atlas, Aseprite export JSON and selected FPG subset | Native loaders and explicit ownership | Shared loaders with userdata descriptors | Optional `assets`; owned metadata and borrowed views; `graphics` adds textures |
| Images, sprites, shapes, graph animation, drawing and cameras | Native storage, tick updates and explicit draw pass | Shared native pass with live descriptor preparation | Optional `graphics`; retained images/sprites, explicit frame process pass |
| Persistent text and fonts | Context registry, retained style and explicit draw pass | Shared registry/pass with live font descriptor updates | Owned text/handles, retained fonts/windows and explicit frame text pass |
| Keyboard, mouse, controller and Unicode text | Native queries and per-tick input snapshot | Same native queries and transitions | Typed queries on an owned window; explicit `input_tick` |
| Sound and music | Explicit device/instance lifetime and shared stream update | Userdata and playback roots; runner pumps native maintenance | Optional `graphics`; RAII device/instances and explicit `Audio::update` |
| Geometric collisions and static rectangle worlds | Native overlap/sweep and process movement | Contextual helpers over native selected colliders | Same native queries and movement through typed shapes/handles |
| Headless use | Core without Lua, Rust or raylib | Headless runner; graphics/audio creation unavailable | Default core crate; graphics linking requires a graphical SDK |
| Project tools and player exports | `new --c`, CMake `game` target and native player | `new`, syntax check and bundled runner | `new --lang rust`, Cargo `game` target and native player |

Ownership is explicit in each language. A raw C resource reference does not keep
its window/device alive. Safe Rust image/font/audio wrappers retain their required
platform owner. Lua retains descriptors and playback roots in its registry.
The shared native manager still controls process/context resource ownership.

C and Rust hosts choose when to tick, pump platform services and draw. Lua's
runner performs those operations. A rendering call does not execute a process.
Sleep/Freeze, graph-before-command order, native animation clocks and process
cleanup use the same engine contracts. Rust's generated starter demonstrates an
explicit host loop; it does not create another scheduler.

Lua aliases, legacy table descriptors, experimental debugger hooks and Lua
`rand` are not extra Rust API styles. Use each language's standard math, random,
file and string libraries where no shared engine service exists. No native
random-stream parity or automatic mixed-language payload conversion is claimed.
C/Lua coexistence is tested internally; public dynamic C/Lua/Rust mixing and
shared user-state conversion are outside this milestone.

See [C contracts](c-contract.md), [Lua processes](lua-processes.md),
[Rust processes](rust-processes.md), [resource lifetime](resource-lifetime.md),
[runner timing](runner-timing.md), [shared rendering](runner-rendering.md),
[audio maintenance](audio-maintenance.md), and [CLI contracts](../specs/cli.md).
