# Proposed versions

The backend, Lua dependency, API, and distribution changes justify a new **2.0** line. The current rockspec contains `1.0.0-0`; it is not changed here and no release is published. The tags below are proposals, not delivered versions.

| Version | Result | Dependency |
| --- | --- | --- |
| Preparation | Review sources and licenses; set baseline and decisions | Current documentation inventory |
| `2.0.0-alpha.1` | C runtime, raylib, bundled Lua, and hello example | Preparation |
| `2.0.0-alpha.2` | C core and API usable without Lua; processes, signals, Lua binding, and verified parity ([in development](2.0.0-alpha.2.md)) | alpha.1 and [C refactor](../architecture/c-first.md) |
| `2.0.0-alpha.3` | Graphics, text, input, and audio in the C API and Lua binding; four ported examples | alpha.2 |
| `2.0.0-alpha.4` | malvado-cli, C SDK, and C/Lua packages for Linux/macOS; Windows path in design | alpha.3 |
| `2.0.0-alpha.5` | Defined and documented essential Bennu profile | alpha.3 and API contracts |
| `2.0.0-alpha.6` | Android prototype and APK/AAB; full Windows validation | alpha.4 and alpha.5 |
| `2.0.0-beta.1` | Frozen essential contract and verifiable documentation | All earlier alphas |
| `2.0.0` | Stable essential profile and accepted exporters | beta and completed criteria |
| `2.1+` | Extended families, FPG variants/export, mountable ZIP, and compatible improvements | Coverage matrix by version |

Define the essential profile before implementing its families. Do not promise 100% Bennu compatibility for 2.0. Documented extended profiles can remain incomplete without blocking 2.0. There are no date estimates until the first cut is complete and format and platform complexity is measured.

## Platform validation sequence

Alphas 1–5 can close with required macOS and Linux builds and tests, without waiting for Android or Windows. Alpha.6 includes the Android window, background/foreground, and bundled Lua prototype, Android tests, and exports. It also adds Windows CI and validates configuration/build, C/Lua and mixed tests, window/input/audio, CLI, exports, and packages on a clean Windows machine. Fix platform failures in alpha.6 before beta. Until then, Android and Windows are targets, not guaranteed support. Validate the GUI on Windows in alpha.6 if it is included in the distribution.

The current workflow covers macOS/Linux. Restore the Windows job in alpha.6. A CI build does not by itself validate installers, graphics, or execution on a clean machine.

## Additional alpha.2 gate: independent C engine

The C core manages processes, signals, and messages. The Lua binding uses the same scheduler. There are headless and windowed C examples and C, Lua, and mixed tests. **Alpha.2 remains under validation** until the current changes pass CI and alpha.1 requirements are complete. See [C architecture](../architecture/c-first.md) for details and limits.

## C and Lua documentation

Alpha.2 includes contract documentation and minimal C and Lua examples. Alpha.3 adds the graphics reference and an initial MkDocs site. Alpha.4 documents the SDK and CLI. Beta completes symbols and examples, and 2.0 publishes versioned documentation. The [documentation proposal](../development/api-documentation.md) defines criteria and tools. The site is not implemented yet.

## CI by version

Alpha.1 adds CMake/CTest builds and documentation checks in GitHub Actions. Alpha.2 adds analysis and sanitizers. Alpha.3 adds graphics smoke tests and the site build. Alpha.4 adds desktop packages and a release candidate. Alpha.6 adds Android and the complete Windows matrix and tests. Beta prepares versioned Pages and reviewed publication. See [CI and releases](../development/ci.md). The earlier build, catalog, sanitizer, and Linux graphics workflow [passed on GitHub](https://github.com/jepemo/malvado/actions/runs/40908427079).

## Optional malvado-gui track

| Milestone | Result | Dependencies |
| --- | --- | --- |
| G0 | Toolkit prototype: ImGui/cimgui versus raygui, DPI, tree, and atlas | Core alpha.3 |
| GUI `0.1.0` / G1 | Projects, external editor, Run/Stop, console, asset preview, and desktop export | Core/CLI alpha.4 and shared services |
| GUI `0.2.0` / G2 | Process inspector, pause, and tick step | G1, core P2 hooks, and validated local protocol |
| GUI `0.3.0` / G3 | Metadata, pivots/points/animation, undo, and available FPG conversion | G2 and asset pipeline |
| GUI `1.0.0` / G4 | Complete flow accepted on supported hosts | G1–G3; Android requires CLI alpha.6 |

The GUI version is independent of the runtime version. None of these deliveries is implemented or blocks the core/CLI release. See [GUI scope](../specs/gui.md) and [formats](../specs/assets.md).

The [alpha.1 status](2.0.0-alpha.1.md) describes the build and remaining work.

## Alpha.1 checklist

- [ ] Resolve the license conflict in existing code.
- [ ] Pin exact Lua and raylib versions, hashes, and notices; avoid implicit downloads after bootstrap.
- [ ] Add CMake, an opaque C API, and a C runner with bundled Lua.
- [ ] Register `require("malvado")`, `process`, `frame`, and minimal startup/shutdown.
- [ ] Keep coroutine registry references and close without leaks.
- [ ] Separate tick and draw; allow an injected clock for headless tests.
- [ ] Run hello on macOS and Linux; validate the Windows smoke test in alpha.6.
- [ ] Show script errors with tracebacks and a nonzero exit code.
- [ ] Document build and execution without installed LÖVE or Lua.
- [ ] Run CI builds on available desktop platforms, CTest with timeouts, and catalog/documentation checks.

## LuaRocks

The 1.0 rockspec remains at the root. A 2.0 rock is an additional option that requires a tested C module for external Lua. The main distribution will be the standalone SDK. See [distribution](../development/distribution.md). CLI and GUI have separate channels and are not automatically included in the rock.

## Version policy

The runtime, Lua binding, and CLI initially use a coordinated version to reduce combinations. The optional GUI has its own version and declares supported project, runtime, and diagnostic protocol versions. The project format has `schema=1`. A documentation version of 2.0 does not change that schema. Keep `main` for development, documentation for each stable minor, and Git tags for exact source revisions. The changelog contains only delivered changes. Keep the roadmap separate.
