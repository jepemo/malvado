# Malvado 2 migration plan

Status: technical proposal. [Alpha.1](../versions/2.0.0-alpha.1.md) and [alpha.2](../versions/2.0.0-alpha.2.md) are complete; later phases are planned. Updated: 2026-10-30.

## Goal

Replace LÖVE with raylib and move the runtime, scheduler, process data, assets, rendering, input, sound, collision, and API functions to C. Create an engine library and public C API usable by C games without Lua. Include the official Lua binding and a runner with bundled Lua. Lua games must not require installed Lua, LÖVE, or LuaRocks. Add `malvado-cli` to create projects and export to Linux, Windows, macOS, and Android. Add `malvado-gui` as an optional desktop workshop for projects, assets, and process inspection, using the same C services.

This proposal replaces the old plan that required “No C changes,” third-party raylib bindings, and LuaRocks as the main path. The old plan remains in [history/PLAN-love-to-raylib-lua.md](../history/PLAN-love-to-raylib-lua.md).

## Verified starting point

- `malvado/core.lua` creates a coroutine per instance, implements `frame()` with `coroutine.yield()`, and resumes from `love.draw`.
- Modules directly call `love.graphics`, `love.window`, `love.timer`, `love.keyboard`, `love.mouse`, and `love.filesystem`.
- Simulation and drawing are mixed; the same process collection is sorted by `z`. The new architecture separates execution order from draw order.
- `collision()` checks whether another process origin is inside a box; it is not yet full sprite collision.
- There are hello world, menu demo, asteroids, and benchmark examples. Asteroids and benchmark also call `love.*` directly, so migrating the library alone is not enough.
- `load_fpg()` contains experimental ZIP work; `fpg()` accepts directories/spritesheets. Do not assume classic binary FPG support.
- The rockspec declares `1.0.0-0` and Lua >= 5.1. This does not prove a complete 1.0 distribution exists.
- The historical Lua engine used GPLv3+ notices while its rockspec declared Apache 2.0. The owner selected Apache 2.0 for the rewrite. The historical engine and rockspec are removed from the current tree and remain in Git history with their notices. Review third-party asset provenance before release.

## Proposed decisions

| Topic | Proposal | Reason |
| --- | --- | --- |
| Implementation | C11 + CMake; raylib behind a C adapter | Native core and desktop/Android builds |
| Game languages | C core with first-class C, Lua, and planned Rust APIs | C games without a VM and Lua games with a standalone runtime |
| Initial Lua version | Evaluate Lua 5.4 as a conservative base; confirm patch and support when dependencies are pinned | C coroutine and cleanup APIs; do not promise LuaJIT/5.1 |
| Processes | One C scheduler; stateful C callbacks and Lua coroutines managed by the binding | Languages share lifecycle, signals, and priorities |
| Reference | BennuGD2 primary, classic BennuGD historical, DIV/Fénix for comparison | Verifiable inventories without mixing generation signatures |
| Compatibility | Malvado 2.0 does not require 1.0 API compatibility; assess external formats separately | Choose Lua behavior for 2.0; the C API is new |
| Distribution | Executable with Lua/raylib; optional C Lua module later | No dependency on the player's environment |
| Tools | `malvado-cli` in C, shared packaging library | C-first services and reusable project logic |
| GUI | Project/asset/process workshop; evaluate ImGui+cimgui+raylib adapter | C services/core; C++ limited to UI dependencies; raygui as a strict C alternative |
| Assets | Editable PNG+JSON, FPG for compatibility, ZIP as container | Preserve IDs/points without a new proprietary graphics binary |
| CI and releases | GitHub Actions for builds, tests, analysis, docs, and version artifacts | Detect PR regressions and create verifiable candidates |
| Documentation | C, Lua, and Rust APIs with runnable examples; Astro/Starlight migration planned | Two first-class references, PR review, and versioned search |

Do not reimplement the Lua VM. Lua is already implemented in C. Do not use operating-system threads to simulate DIV processes. The Lua VM retains a script stack even with a C scheduler. The C core must run without Lua using native processes with explicit state. It does not promise arbitrary C stack suspension. See [C architecture and Lua binding](../architecture/c-first.md).

## Phases and exit gates

| Phase / proposed version | Work | Evidence required to close |
| --- | --- | --- |
| P0 / preparation | Resolve licenses; pin Lua/raylib sources, hashes, and versions; measure current examples; select legacy behavior | Classified API matrix, baseline captures/traces, reproducible dependency build |
| P1 / 2.0.0-alpha.1 | CMake, bundled Lua, C context, raylib window, runner, `process/frame`, hello, initial build/CTest CI | Hello opens/closes without LÖVE or installed Lua on macOS/Linux; scheduler works headless |
| P2 / 2.0.0-alpha.2 | Extract scheduler, processes, signals, messages, and public C API; connect official Lua binding; inspection and both API docs | C game without Lua, C/Lua parity, examples/contracts, deterministic traces, released references, memory tests |
| P3 / 2.0.0-alpha.3 | Current 2D graphics, input, text, audio, examples, and development workflow | Tagged preview; unchecked validation items remain open |
| P4 / 2.0.0-alpha.4 | Minimal `malvado` client, C SDK, project creation, execution, and desktop export | Automated create, run, export, and execution checks for C and Lua on macOS/Linux |
| P5 / 2.0.0-alpha.5 | Process identity, lifecycle, ownership, resources, genealogy, and C/Lua API migration | Contracts and automated lifecycle, graphics, resource, stress, and C/Lua parity checks in [alpha.5](../versions/2.0.0-alpha.5.md) |
| P6 / 2.0.0-alpha.6 | Simple Lua 2D APIs, unified assets, automatic animation, contextual world calls, main camera and following; bounded inventory decisions | A6-01–12 contracts, examples, native/headless tests, graphics checks and candidate CI in the alpha.6 plan |
| P7 / 2.0.0-alpha.7 | Rust SDK and multi-language architecture | Shared scheduler, installed SDK, parity and Rust game checks |
| P8 / 2.0.0-alpha.8 | Complete all remaining agreed 2.0 API families in C, Lua, and Rust | Every in-scope API entry has an implementation, contract, and behavior test |
| P9 / 2.0.0-alpha.9 | Windows and Android runtimes, packaging, and exports | Automated Windows CI and Android emulator checks where supported |
| P10 / 2.0.0-alpha.10 | Complete public documentation, examples, site, packaging, and platform review | Published site and automated end-to-end checks for supported targets |
| P11 / 2.0.0-beta.1 | Freeze the 2.0 API, manually test user flows, and prepare release candidates | Complete inventory, API parity, documentation, manual device and clean-host results |
| P12 / 2.0.0 | Stabilization and release | Beta gates complete and release artifacts verified |
| 2.1 | Mode 7 | Perspective floor, sprites, C/Lua APIs, examples and tests |
| 2.2 | Mode 8 | Sector world, collision, C/Lua APIs, examples and tests |
| 2.3 | Mode 9 | 3D worlds and sprites, C/Lua APIs, examples and tests |
| 2.4 | Optional GUI | Project and asset workflow using existing services |
| 2.x, later | Evaluate LuaJIT | Compare real Malvado workloads, compatibility, memory, and supported-platform behavior before deciding |

The complete **Malvado 2.0 API** is a release gate, except graphics modes 7, 8 and 9. Define its exact inventory and contracts before implementation. Do not silently equate that inventory with every historical DIV or Bennu export: assess the references and record differences with the owner. Earlier claims that only the essential profile is required no longer set the 2.0 scope.

The required targets are macOS, Linux, Windows, and Android. Alphas include automated tests for their features, including graphics, clean runners, and emulators where practical. Manual user-flow, physical-device, and clean-computer testing belongs to beta. Fix beta findings before the stable release. iOS is deferred to a later version; its feasibility, toolchain, signing, distribution, and device testing are not 2.0 release gates. The GUI and graphics modes are not 2.0 gates. Alpha.3 is tagged as a preview, with remaining implementation and automated checks tracked in its version file.

Lua 5.4 remains the 2.0 runtime. A later LuaJIT evaluation must benchmark process scheduling, Lua calculations, rendering workloads, and memory on the same hardware and build settings. Review Lua 5.1/5.4 language and C API differences, coroutine cleanup, build and export support, and platform limitations before choosing whether to add another runtime. This is an investigation, not a commitment to switch.

## API documentation

C, Lua, and Rust will have getting-started guides, module references, and equivalent runnable examples. The initial site uses Markdown in Git, MkDocs, and GitHub Pages. The plan uses C signatures in public headers and LuaCATS definitions for the binding. See the [documentation plan](../development/api-documentation.md) for sources, CI, and versioning. Full site references remain pending.

## CI and release publication

The [GitHub Actions plan](../development/ci.md) starts in alpha.1 with available-platform builds, headless tests, and catalog verification. Add static analysis, sanitizers, and graphics smoke tests in later phases. Prepare candidate binaries and draft releases when packaging exists. Document and publish Pages through a separate job. State current checks and pending platforms explicitly.

## Graphics tools track

See [malvado-gui](../specs/gui.md). G0 validates toolkit/DPI/tables/trees. G1 creates projects, opens an external editor, runs, and exports. G2 inspects processes and controls ticks. G3 edits metadata/animations and reuses FPG importers. The GUI is optional and planned for 2.4. It does not block runtime 2.0. The first delivery runs the game in a separate process and window; it does not require a full IDE or embedded preview.

See [assets](../specs/assets.md) for formats and distribution: PNG+JSON during development, FPG interoperability by variant, and ZIP for transport/distribution. FPG export and runtime-mountable ZIP are later capabilities, separate from the first FPG reader.

## Proposed code layout

```text
src/include/malvado/   C API and opaque types
src/core/              processes, scheduler, messages, clock, assets
src/modules/           graphics, text, input, sound, and utility functions
src/platform/raylib/   window, GPU, audio, and platform integration
src/bindings/lua/      C API registration and userdata
src/tooling/           shared C services for projects, assets, and export
src/cli/               command interface to those services
src/gui/               UI, preferences, and isolated graphics adapter
src/debug/             snapshots/commands and opt-in local protocol
lua/malvado/           minimal facade and optional compatibility
build/_deps/           CMake-downloaded dependencies, ignored by Git
tools/cmake/           build and package support
platform/android/      Activity/Gradle/NDK template
examples/              runnable games and assets
api/                   Malvado contract when formalized
```

This layout is a proposal; do not create empty code directories for it. Research stays in `docs/api`, with external originals isolated from the build.

## Risks and mitigation

- **Process semantics:** specify them before porting, with execution-order and timing traces. Do not carry old-core bugs into the new profile by accident.
- **Data between Lua and C:** userdata with generation, ownership, and registry references; never use raw memory pointers as public IDs.
- **Raylib does not cover the full DIV family:** FPG/MAP/FNT/PAL, mask collision, scroll, and Mode 7 need custom logic.
- **Incomplete historical documentation:** exports prove names and types, not argument names or behavior. Read implementations and examples before completing each record.
- **GUI scope:** validate the focused workshop first; do not turn migration into a full IDE. ImGui C++ dependencies stay outside runtime/CLI and games.
- **Asset fidelity:** retain IDs, points, indexes/palettes, and offsets. PNG RGBA alone does not guarantee historical semantics.
- **Multiplatform:** use target toolchains, native smoke tests, and a pinned Android SDK. Do not promise universal cross-compilation from one machine.

## Recommended next implementation

Continue with the API inventory in [alpha.6](../versions/2.0.0-alpha.6.md). See [versions](../versions/README.md) for the checklist and [CLI](../specs/cli.md) for later exporters.
