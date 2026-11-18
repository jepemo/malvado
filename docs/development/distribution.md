# Distribution: runtime, tools, and LuaRocks

Status: in progress. No 2.0 release package has been published. This document distinguishes the distributed products and the work needed to test them.

An initial local SDK archive can be built with `mise run package-sdk`. It writes a `.tar.gz` file and a SHA-256 checksum under `build/sdk-package/`. The task builds for the current host and refuses to overwrite an existing archive. It is a development artifact, not a validated release or a game export. Move or remove the previous output before running it again. See [alpha.4](../versions/2.0.0-alpha.4.md) for verification status.

On a push or manual CI run, the `sdk-candidate` artifact is available only after all CI jobs pass. It contains the tested macOS and Linux archives and their checksums. Its name includes the source commit. GitHub retains it for 14 days. This is a development candidate, not a release. The remaining platform and release checks in the version checklist still apply.

## Source archive

From a Git checkout, use Python 3.11 or later and Git 2.38 or later to package
the selected commit. Use a new output directory for each run.

```sh
python3 tools/package_source.py HEAD build/source-package
```

The command writes one platform-independent `malvado-source-<commit>.tar.gz`
and a SHA-256 checksum. The archive contains the tracked source tree, including public
C headers, the core, assets, platform code, bindings, CLI, build files, tests,
examples and license notices. The top-level `SOURCE-REVISION` file records the
full commit ID. Uncommitted changes and local build outputs stay outside the
archive. Building the extracted tree does not require a Git checkout.

The core-only configuration with Lua, raylib and assets disabled needs no
dependency downloads. A fresh default build needs network access for Lua
5.4.8 from lua.org and cJSON 1.7.19 and raylib 6.2 from GitHub. CMake checks the
SHA-256 pins in [CMakeLists.txt](../../CMakeLists.txt). The optional GUI
prototype uses additional pinned downloads from GitHub. Platform and export
toolchains are installed separately. See the [platform matrix](alpha8-platforms.md).

Verify the archive against the full commit ID and keep the report with the
archive and checksum. Use a host with CMake, a C compiler and the desktop
graphics build dependencies.

```sh
revision=$(git rev-parse HEAD)
python3 tools/verify_source.py "build/source-package/malvado-source-$revision.tar.gz" \
  "$revision" --report build/source-package/verification.json
```

The verifier extracts the archive to a clean directory. It builds and tests
the dependency-free core, then builds and installs the default desktop SDK.
The installed SDK test compiles and runs external C consumers against the
public API. The verifier also checks the installed Malvado, Lua, cJSON and
raylib notices against their source files. The CI job **Verify source archive**
uses the same commands on Ubuntu and retains the archive, checksum, command
log and verification report in `source-check-<commit>`.

## Proposed decision

The main Malvado 2 channel will be a **downloadable SDK per platform**. The development SDK includes C libraries and headers, the `malvado` client, and the Lua runtime. The client creates minimal C and Lua projects. The GUI will have a separate desktop download or optional SDK package. Exported Lua games include the runtime and Lua. C games include their native runtime without requiring Lua. Players do not need to install Lua or LuaRocks.

Evaluate an **optional** LuaRocks distribution for developers who already use Lua. This requires a tested C module that can load in an external interpreter. The current `malvado-run` has its own VM and registers `malvado` in it. This does not automatically make it a module that any `lua` can load. Do not advertise `luarocks install malvado` for 2.0 until that module and its rockspec work.

## Historical 1.0 rockspec

The historical 1.0 rockspec described the Lua/LÖVE engine and referred to files removed from the Malvado 2 tree. It remains available with that engine in the [historical revision](https://github.com/jepemo/malvado/tree/4670d8a). It does not describe or install Malvado 2. Removing it from the current tree does not change previously published packages.

## Products and channels

| Product | Main channel | LuaRocks | Audience |
| --- | --- | --- | --- |
| Core and C API | SDK with headers and native library per platform | No LuaRocks or Lua required | C games and tools |
| Lua runtime | Bundled with the client and exported Lua games | Optional; define a separate package only if useful | Game players and developers |
| Lua `malvado` binding | Included in runner | Optional rock with a C module for external Lua; design pending | Projects that already manage Lua/LuaRocks |
| `malvado` client | SDK/release archive per platform | Possible separate rock later, after executable and dependency installation are validated | Project creators and exporters |
| `malvado-gui` | Separate desktop app or optional SDK distribution | Not a priority; toolkit and native packaging make this path harder | Creators who prefer a GUI |
| Exported game | Desktop ZIP/folder, Android APK/AAB | Not required for the player | Player |

Separate rock names such as `malvado` and `malvado-cli` are proposals. Check availability and compatibility policy before publication. A Lua library rock does not necessarily install the full SDK, exporters, or GUI. An offline or target-specific SDK must list its binaries and dependencies in a manifest.

## Pending technical decision for a 2.0 rock

An external module must link to the ABI of the interpreter that loads it and be tested with the supported Lua version. The initial proposal is Lua 5.4. Do not assume 5.1/LuaJIT compatibility or load a second independent Lua copy in the same module. The standalone runner links the Lua built for distribution. A C game links the core without Lua. The native game and binding must share the C core. Separate host/module registration from the executable; do not maintain two implementations of engine rules.

The [rockspec format](https://github.com/luarocks/luarocks/blob/main/docs/rockspec_format.md) supports `builtin`, `cmake`, and other backends, C modules, external dependencies, and file installation. Select a backend after adding module targets and installation rules to CMake. Do not add a 2.0 rockspec before a loadable external module exists. Decide whether raylib is built with the module or declared as an external dependency. Pin versions and platform requirements.

The external binding must define window and loop ownership, initialization and shutdown, and behavior in another graphics host. Do not assume it works inside LÖVE or with another loop. The Lua contract must state host differences. Linux, macOS, and Windows require module-load and real-run tests, `luarocks lint`, clean-tree installation, and uninstall tests.

To avoid unexpected updates to existing games, versions 1.x and 2.x must declare their version ranges and migration guide. If both use `malvado`, 2.0 must identify the API and engine break. Before publication, decide how to select 1.x and whether another name is useful. Verify actual LuaRocks resolver behavior and name availability.

SDK and game package checks must use the [current process and startup contracts](../api/process-inventory.md). Installed C headers and libraries must provide the owned process handle API. Exported Lua games must support both entry forms. Keep Android emulator checks in alpha.8 and manual clean-machine checks in beta.

## Versions and CI

Runtime and CLI use the proposed coordinated versions. GUI has its own version and compatibility matrix. Release artifacts record OS, architecture, runtime/Lua/raylib versions, hashes, and notices. When a rock exists, generate its rockspec from a pinned tag and run `luarocks lint` and `luarocks make` in temporary trees on each supported platform. Test the source artifact (`luarocks pack`). If binary rocks are published, test them by platform and Lua ABI. Publish to LuaRocks only after these tests and license resolution, as part of the release process. Upload requires an account and credentials and is separate from CI candidate generation.

No 2.0 rockspec or LuaRocks installation targets are implemented. The initial CLI is under development; the GUI remains planned. The [CI plan](ci.md) and [roadmap](../versions/README.md) record these dependencies.
