# Malvado 2 migration plan

Status: technical proposal. No implementation phase is declared complete. Updated: 2026-10-29.

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
| Game languages | Full C API and official Lua binding, both first-class | C games without a VM and Lua games with a standalone runtime |
| Initial Lua version | Evaluate Lua 5.4 as a conservative base; confirm patch and support when dependencies are pinned | C coroutine and cleanup APIs; do not promise LuaJIT/5.1 |
| Processes | One C scheduler; stateful C callbacks and Lua coroutines managed by the binding | Languages share lifecycle, signals, and priorities |
| Reference | BennuGD2 primary, classic BennuGD historical, DIV/Fénix for comparison | Verifiable inventories without mixing generation signatures |
| Compatibility | Explicit legacy Malvado and DIV-family profiles | Degrees, scale, text, and timing already differ |
| Distribution | Executable with Lua/raylib; optional C Lua module later | No dependency on the player's environment |
| Tools | `malvado-cli` in C, shared packaging library | C-first services and reusable project logic |
| GUI | Project/asset/process workshop; evaluate ImGui+cimgui+raylib adapter | C services/core; C++ limited to UI dependencies; raygui as a strict C alternative |
| Assets | Editable PNG+JSON, FPG for compatibility, ZIP as container | Preserve IDs/points without a new proprietary graphics binary |
| CI and releases | GitHub Actions for builds, tests, analysis, docs, and version artifacts | Detect PR regressions and create verifiable candidates |
| Documentation | C and Lua APIs with runnable examples, Markdown, and MkDocs on Pages | Two first-class references, PR review, and versioned search |

Do not reimplement the Lua VM. Lua is already implemented in C. Do not use operating-system threads to simulate DIV processes. The Lua VM retains a script stack even with a C scheduler. The C core must run without Lua using native processes with explicit state. It does not promise arbitrary C stack suspension. See [C architecture and Lua binding](../architecture/c-first.md).

## Phases and exit gates

| Phase / proposed version | Work | Evidence required to close |
| --- | --- | --- |
| P0 / preparation | Resolve licenses; pin Lua/raylib sources, hashes, and versions; measure current examples; select legacy behavior | Classified API matrix, baseline captures/traces, reproducible dependency build |
| P1 / 2.0.0-alpha.1 | CMake, bundled Lua, C context, raylib window, runner, `process/frame`, hello, initial build/CTest CI | Hello opens/closes without LÖVE or installed Lua on macOS/Linux; scheduler works headless |
| P2 / 2.0.0-alpha.2 | Extract scheduler, processes, signals, messages, and public C API; connect official Lua binding; inspection and both API docs | C game without Lua, C/Lua parity, examples/contracts, deterministic traces, released references, memory tests |
| P3 / 2.0.0-alpha.3 | PNG+JSON render/assets, input, text, audio in C and Lua; legacy shim; migrate four examples; prepare GUI G0 | Equivalent C/Lua demos without `love.*`, comparable images, documented benchmark |
| P4 / 2.0.0-alpha.4 | CLI, C SDK, Lua runner, desktop packages; enable GUI G1 | Create → play → export → run C and Lua projects on macOS/Linux; Windows in alpha.6 |
| P5 / 2.0.0-alpha.5 | Essential Bennu compatibility, scrolling, regions, first pinned-variant FPG importer, basic Aseprite adapter, collision, selected extras | Each essential-profile symbol has a contract, example, and semantic check |
| P6 / 2.0.0-alpha.6 | Android export and full Windows validation: runtime, C/Lua API, CLI, packaging | Android APK/AAB, touch, lifecycle; Windows CI, C/Lua examples, CLI, clean-machine packages; fix failures before beta |
| P7 / 2.0.0-beta.1 | Freeze essential API, publishable docs, packaging, consistent errors | Unambiguous support matrix, complete examples, supported-OS tests |
| P8 / 2.0.0 | Stabilization and migration guide | Beta criteria complete, reproducible releases, accepted sample games |
| 2.1+ | Expand pending historical families and functions | Versions by profile; never declare full compatibility without its completed matrix |

Alphas 1–5 close with macOS and Linux verification. Android and Windows prototypes, tests, and fixes are concentrated in alpha.6 before beta. Both remain product targets but are not announced as supported before validation. The first stable version is not blocked by CD-ROM, raw pointers, advertising, or historical functions with little value. List them as pending or unsupported with a reason. Researching the full API is separate from implementing it in one delivery.

## API documentation

C and Lua will have getting-started guides, module references, and equivalent runnable examples. The proposal uses Markdown in Git, MkDocs, and GitHub Pages, with C signatures in public headers and LuaCATS definitions for the binding. See the [documentation plan](../development/api-documentation.md) for sources, CI, and versioning. The proposed site is not implemented yet.

## CI and release publication

The [GitHub Actions plan](../development/ci.md) starts in alpha.1 with available-platform builds, headless tests, and catalog verification. Add static analysis, sanitizers, and graphics smoke tests in later phases. Prepare candidate binaries and draft releases when packaging exists. Document and publish Pages through a separate job. State current checks and pending platforms explicitly.

## Graphics tools track

See [malvado-gui](../specs/gui.md). G0 validates toolkit/DPI/tables/trees. G1 creates projects, opens an external editor, runs, and exports. G2 inspects processes and controls ticks. G3 edits metadata/animations and reuses FPG importers. The GUI is optional, has its own version, and does not block runtime 2.0. The first delivery runs the game in a separate process and window; it does not require a full IDE or embedded preview.

See [assets](../specs/assets.md) for formats and distribution: PNG+JSON during development, FPG interoperability by variant, and ZIP for transport/distribution. FPG export and runtime-mountable ZIP are later capabilities, separate from the first FPG reader.

## Proposed code layout

```text
include/malvado/       C API and opaque types
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
cmake/                 dependencies/toolchains
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

Continue alpha.2 with the [C refactor](../architecture/c-first.md): a C context and game without Lua, native scheduler, and Lua binding connected to the same scheduler. Preserve existing Lua tests and behavior. Resolve open alpha.1 requirements before closing. See [versions](../versions/README.md) for the checklist and [CLI](../specs/cli.md) for exporters.
