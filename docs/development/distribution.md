# Distribution: runtime, tools, and LuaRocks

Status: proposal. No 2.0 package or tools have been published. This document distinguishes the distributed products and the work needed to test them.

## Proposed decision

The main Malvado 2 channel will be a **downloadable SDK per platform**. It will include the C library and headers, Lua binding/runner, and, when available, `malvado-cli` and templates. The GUI will have a separate desktop download or optional SDK package. Exported Lua games include the runtime and Lua. C games include their native runtime without requiring Lua. Players do not need to install Lua or LuaRocks.

Evaluate an **optional** LuaRocks distribution for developers who already use Lua. This requires a tested C module that can load in an external interpreter. The current `malvado-run` has its own VM and registers `malvado` in it. This does not automatically make it a module that any `lua` can load. Do not advertise `luarocks install malvado` for 2.0 until that module and its rockspec work.

## Historical 1.0 rockspec

The historical 1.0 rockspec described the Lua/LÖVE engine and referred to files removed from the Malvado 2 tree. It remains available with that engine in the [historical revision](https://github.com/jepemo/malvado/tree/4670d8a). It does not describe or install Malvado 2. Removing it from the current tree does not change previously published packages.

## Products and channels

| Product | Main channel | LuaRocks | Audience |
| --- | --- | --- | --- |
| Core and C API | SDK with headers and native library per platform | No LuaRocks or Lua required | C games and tools |
| `malvado-run` / runtime | SDK or desktop binary with bundled Lua | Optional; define a separate package only if useful | Game players and developers |
| Lua `malvado` binding | Included in runner | Optional rock with a C module for external Lua; design pending | Projects that already manage Lua/LuaRocks |
| `malvado-cli` | SDK/release archive per platform | Possible separate rock later, after executable and dependency installation are validated | Project creators and exporters |
| `malvado-gui` | Separate desktop app or optional SDK distribution | Not a priority; toolkit and native packaging make this path harder | Creators who prefer a GUI |
| Exported game | Desktop ZIP/folder, Android APK/AAB | Not required for the player | Player |

Separate rock names such as `malvado` and `malvado-cli` are proposals. Check availability and compatibility policy before publication. A Lua library rock does not necessarily install the full SDK, exporters, or GUI. An offline or target-specific SDK must list its binaries and dependencies in a manifest.

## Pending technical decision for a 2.0 rock

An external module must link to the ABI of the interpreter that loads it and be tested with the supported Lua version. The initial proposal is Lua 5.4. Do not assume 5.1/LuaJIT compatibility or load a second independent Lua copy in the same module. The standalone runner links the Lua built for distribution. A C game links the core without Lua. The native game and binding must share the C core. Separate host/module registration from the executable; do not maintain two implementations of engine rules.

The [rockspec format](https://github.com/luarocks/luarocks/blob/main/docs/rockspec_format.md) supports `builtin`, `cmake`, and other backends, C modules, external dependencies, and file installation. Select a backend after adding module targets and installation rules to CMake. Do not add a 2.0 rockspec before a loadable external module exists. Decide whether raylib is built with the module or declared as an external dependency. Pin versions and platform requirements.

The external binding must define window and loop ownership, initialization and shutdown, and behavior in another graphics host. Do not assume it works inside LÖVE or with another loop. The Lua contract must state host differences. Linux, macOS, and Windows require module-load and real-run tests, `luarocks lint`, clean-tree installation, and uninstall tests.

To avoid unexpected updates to existing games, versions 1.x and 2.x must declare their version ranges and migration guide. If both use `malvado`, 2.0 must identify the API and engine break. Before publication, decide how to select 1.x and whether another name is useful. Verify actual LuaRocks resolver behavior and name availability.

## Versions and CI

Runtime and CLI use the proposed coordinated versions. GUI has its own version and compatibility matrix. Release artifacts record OS, architecture, runtime/Lua/raylib versions, hashes, and notices. When a rock exists, generate its rockspec from a pinned tag and run `luarocks lint` and `luarocks make` in temporary trees on each supported platform. Test the source artifact (`luarocks pack`). If binary rocks are published, test them by platform and Lua ABI. Publish to LuaRocks only after these tests and license resolution, as part of the release process. Upload requires an account and credentials and is separate from CI candidate generation.

No 2.0 rockspec, LuaRocks installation targets, CLI, or GUI are implemented. The [CI plan](ci.md) and [roadmap](../versions/README.md) record these dependencies.
