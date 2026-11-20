# WebAssembly and HTML5 export

Status: feasibility proposal, researched on 2026-11-16. See
[selecting work](README.md#promotion-into-a-release) for how this proposal can enter
a release.

## Outcome and feasibility

Export a game with the existing `malvado` CLI to a folder served over HTTP(S),
including static hosting, GitHub Pages and itch.io. Keep gameplay APIs and the
shared C process scheduler consistent with desktop. Platform differences must
have documented errors and limits. No separate web engine or scheduler.

The source review supports a feasible C/Lua path, but this is an engineering
assessment, not a tested port. Rust needs a separate toolchain proof.

| Current evidence | Consequence for the proposed port |
| --- | --- |
| `CMakeLists.txt` separates `malvado_core`, platform adapters and Lua; raylib is pinned to 5.5 and Lua to 5.4.8 | Cross-compile the same sources with Emscripten; preserve C-only builds without Lua or raylib. Review target flags, POSIX definitions, libraries and host-only install/tests rather than assuming a toolchain switch is sufficient. |
| `src/platform/raylib.c` implements backend operations and resource cleanup | Reuse raylib's web backend. Keep browser glue at the platform/runner boundary and GPU resources on the owner thread. |
| `bindings/lua/runtime.c` resumes Lua coroutines through the shared process runtime, but `start` runs a blocking loop with stack-local timing state | Retain scheduler semantics; make frame hosting browser-driven and keep callback state alive through shutdown. Lua state and resources cannot be destroyed when native `main` returns. |
| `src/core/clock.c` and `src/core/process.c` provide timing and scheduling | Reuse bounded stepping, signals, messages and cleanup. Browser animation callbacks drive frames; they do not schedule game processes independently. |
| `src/cli.c`, `src/build.c` and `src/export.c` separate project dispatch, builds and export | Extend these paths with target selection and a web toolchain, without a separate user tool. Current exports target the host. |
| `bindings/rust/malvado-sys/build.rs` rejects cross-compilation and probes an installed native SDK; the safe adapter uses owned futures | A Rust target needs cross-SDK discovery and validation, not just a Cargo flag. Keep future polling inside the C scheduler. |

See [architecture](../development/architecture.md), [CLI](../development/cli.md),
[resource lifetime](../api/resource-lifetime.md) and
[graphics modes](graphics-modes.md) for existing boundaries.

## Runtime and platform approach to evaluate

Compile the core, assets, raylib adapter and optional Lua library using one
pinned Emscripten SDK and CMake toolchain. Build host CLI tools with the host
compiler; do not try to run WASM build utilities as native programs.
[Raylib's web guide](https://github.com/raysan5/raylib/wiki/Working-for-Web-%28HTML5%29)
provides the starting integration path. Verify it against the pinned raylib
release before choosing flags or upgrading a dependency.

Prefer a callback that advances the existing clock, ticks processes, maintains
audio and renders once, then returns to the browser. Define initialization,
first tick, pause, background-tab timing, errors, cancellation and exactly-once
cleanup before implementation. Desktop and web must share that frame behavior.
[Emscripten's runtime guide](https://emscripten.org/docs/porting/emscripten-runtime-environment.html)
describes callback loops and Asyncify. Evaluate Asyncify only if preserving a
blocking entry contract requires it; measure its cost and Lua unwind behavior.
Do not silently change when `mv.start()` returns or how errors reach the runner.
Long-running process callbacks still block the page and must yield cooperatively.

Select WebGL 1 or 2 and the required OpenGL ES shader profile during W1. Desktop
OpenGL shaders, extensions, texture limits and framebuffer formats are not
portable guarantees. Use raylib operations and explicit capability errors;
validate custom effects when those features exist. See
[Emscripten OpenGL support](https://emscripten.org/docs/porting/multimedia_and_graphics/OpenGL-support.html).

Lua should remain the bundled interpreter compiled into WASM with the existing
bindings and coroutine model. Package source scripts and preserve project-relative
`require` paths. The [Lua manual](https://www.lua.org/manual/5.4/manual.html#6.3)
describes module loading; desktop shared libraries cannot be loaded in the web
build. Initially reject dynamic native modules. Any later static module must be
rebuilt for the same toolchain and explicitly included. Audit `io`, `os`, `package`,
`loadfile`, subprocesses and native loaders; virtual files are not host files.
No LuaJIT or remote script service is required.

Rust should first evaluate `wasm32-unknown-emscripten`, installed with rustup,
with Cargo linking the same Emscripten-built engine and raylib libraries.
[The Rust target guide](https://doc.rust-lang.org/rustc/platform-support/wasm32-unknown-emscripten.html)
warns about ABI differences across Emscripten versions and flags. Pin Rust,
Emscripten, standard-library build and linker settings together; assess whether
matching prebuilt `std` is sufficient or a nightly `build-std` is needed.
`wasm32-unknown-unknown` would require extra libc/graphics integration; WASI alone
does not provide browser graphics. Neither is a drop-in replacement. Check
32-bit layouts, callbacks, panic policy, clocks, ownership, process cancellation
and resource release. No second async executor. Preserve SDK mismatch rejection
before unsafe calls, including target and configuration checks.

## Packaging, persistence and CLI

Use a preloaded virtual filesystem first. Emscripten supports packaging files
before program startup, so existing synchronous loaders can read local virtual
paths. See [filesystem overview](https://emscripten.org/docs/porting/files/file_systems_overview.html).
The exporter must wait for preload completion and report missing/corrupt files.

- Include images, sprites, atlas manifests and referenced images, supported FPG,
  fonts, sound/music, Lua modules and game configuration. Preserve relative paths
  and filename case. Keep current format limits; web export adds no FPG variants.
- Define explicit inclusion/exclusion rules. Start with a bounded project asset
  set rather than inferring all dependencies from dynamic Lua strings. Validate
  paths, reject traversal/symlinks and exclude build output, SDKs and secrets.
- Package declarative game tables already supported by Lua. Package future scene
  files and 3D assets only after their runtime formats are delivered. Include
  referenced textures/materials and reject unsupported formats explicitly.
- Separate immutable bundled assets from writable save data. MEMFS writes are
  temporary. Evaluate IDBFS/IndexedDB in W3 with asynchronous restore before game
  startup and explicit flush completion/errors. Specify save identity, schema,
  quota/denial behavior and migration. Browser storage can be cleared or evicted;
  closing a tab cannot guarantee a final asynchronous save.

The likely split is `build --target web` for compilation,
`export --target web` for the deployable folder, and `run --target web` for a
build plus a local HTTP server. These are sketches: today's CLI has no such
options, and `build` currently serves C/Rust projects. Decide Lua build semantics,
project selection, destination arguments and error codes in W2. Reuse existing
safe export staging, destination refusal and license copying. Keep target caches
separate from native `.malvado/build`; report missing/incompatible emsdk through
`doctor` and command failures. No implicit toolchain downloads.

An illustrative folder contains `index.html`, JavaScript glue, a `.wasm` module,
an optional `.data` bundle and notices. Final names, shell and layout remain
open. Use relative URLs that work under hosting subpaths and itch.io iframes.
Document WASM MIME types, HTTP(S), loading/error UI and cache consistency. The
local server needs a bounded document root, clean shutdown and port errors.
A server backend, service worker and hosting account are not prerequisites.

## Implementation phases and acceptance gates

These phases describe dependencies. Before implementation, define a bounded
scope and pin the browser and toolchain matrix. Automated browser checks
start in W1.

### W1 — experimental C web runtime

Depends on completed 2.0 platform validation. Excludes CLI distribution, Lua,
Rust, persistent saves and future graphics modes.

- [ ] Cross-build the C core without Lua/raylib and a separate graphical raylib
  build with a pinned emsdk. Run scheduler, timing and cleanup behavior checks
  in WASM; native C-only tests must still pass.
- [ ] Deliver a minimal C 2D example under `examples/` with shared assets and a
  README. Automated Chromium checks over HTTP must observe rendering, keyboard/
  mouse input, bounded ticks, tab pause/resume and clean stop/error handling.
  Compare deterministic scheduler traces with native execution.
- [ ] Record callback lifetime and entry behavior decisions, WebGL baseline,
  memory/startup measurements and unsupported APIs. Fail loading clearly if
  the required WebGL context or an asset is unavailable.

This is an explicitly incomplete language example until C/Lua/Rust alignment
passes the example policy. No claim of general web export support.

### W2 — deployable C/Lua export through the CLI

Depends on W1 and a resolved Lua entry/lifetime design. Does not wait for Rust,
an editor, scene authoring or Modes 7/8/9.

- [ ] Extend existing build/export/run dispatch and doctor with the selected web
  contract. Clean-runner tests cover absent tools, invalid targets, target cache
  isolation, destination refusal, staging failure and local-server shutdown.
- [ ] Compile bundled Lua and bindings; verify `mv.start()` and returned game
  tables, `require`, signals, timing, process failure and cleanup against native
  traces. Unsupported native modules and OS operations produce documented errors.
- [ ] Export a complete C/Lua 2D game with shared assets, configuration, fonts,
  atlas/FPG and audio fixtures using the supported formats. Verify asset loading,
  corrupt/missing resources, path case, notices and safe inclusion rules.
  Add automated audio activation/playback checks with an injected user gesture;
  document any unverified codecs. Temporary writes are explicitly nonpersistent.
- [ ] Automated Chromium and Firefox tests serve only the exported folder under
  a non-root URL and an iframe. Check loading UI/errors, pixels, controls, focus
  loss and HTTP failures. Document GitHub Pages and itch.io deployment settings.
  The example remains incomplete for Rust until W4; do not label it aligned.

### W3 — browser reliability and capability expansion

Depends on W2. Future graphics checks depend on each mode being delivered;
those modes do not become requirements for completing basic web support.

- [ ] Deliver persistent saves with restore/flush tests across reloads and
  isolated origins; test storage refusal, quota failures and explicit error paths.
- [ ] Validate sound/music pause/resume, audio unlock, focus changes, gamepad
  and touch where claimed. Record unsupported devices and reserved browser keys.
  Run automated Chromium, Firefox and WebKit checks; document version coverage.
- [ ] Measure download size, startup, peak memory and frame time with a fixed
  game. Set budgets before optimizing; compare release builds and retain native
  behavior tests. Test allocation failure and WebGL context loss/recovery policy.
- [ ] Publish a capability matrix for delivered 2D/effects and any delivered
  Modes 7/8/9. Use pixel/depth/clip/shader fixtures for each claimed mode/browser;
  explicitly defer modes that do not exist. Mode 7 needs floor projection and
  horizon checks; Mode 8 needs sector/portal rendering; Mode 9 needs model,
  material, depth and transparency checks. No desktop parity claim by inference.

### W4 — independent Rust web support

Depends on W2 and a successful Rust toolchain/ABI proof; it need not block W3.
If that proof fails, retain an open research item with evidence, rather than
reporting Rust web support complete or changing the process model.

- [ ] Resolve toolchain and standard-library compatibility, cross-SDK probing,
  static link propagation and target/configuration mismatch rejection. Verify an
  external installed-SDK consumer without checkout paths on a clean runner.
- [ ] Export and run the same game through Cargo and the CLI. Compare C/Lua/Rust
  scheduling, owned-future cancellation, errors and cleanup traces. Run the W2
  graphics/input/assets checks for Rust and document remaining platform API gaps.
- [ ] Complete all three example variants and shared README before marking them
  aligned. Preserve native Rust SDK tests and introduce no independent executor.

## Browser limits and decisions to resolve

Start single-threaded. Pthreads require shared memory and deployment headers
(COOP/COEP); this would reduce simple static-host portability. Do not require
threads for gameplay processes. See [Emscripten pthreads](https://emscripten.org/docs/porting/pthreads.html).

Audio generally needs user activation; loading a page is not permission to play.
Plan an accessible start action and test suspended/resumed audio, including music
stream updates. See [Emscripten audio](https://emscripten.org/docs/porting/Audio.html).
Keyboard shortcuts, fullscreen, pointer lock and gamepad access depend on browser
policy and focus; touch is not automatically equivalent to a mouse. Background
tabs may throttle frames. No unlimited catch-up, desktop filesystem access,
subprocesses, native dynamic libraries or guaranteed storage is implied.

WASM memory, asset preload copies, Lua GC and GPU limits need measurement on the
selected browsers, including mobile before claiming mobile support. Do not
promise native throughput or a fixed universal memory ceiling. WebGL is a
restricted graphics environment; desktop shader success is insufficient evidence.
Manual end-user, physical-device and hosted deployment checks supplement the
automated checks when this proposal is implemented.

Before each phase, record decisions for: frame/entry contracts and shutdown (W1,
Lua extension W2); WebGL/shader baseline and context loss (W1/W3); SDK distribution
and supported build hosts (W2); command syntax and Lua builds (W2); asset manifest,
output layout and save identity/flush semantics (W2/W3); browser/device matrix and
performance budgets (W1–W3); Rust target, `std`, ABI and panic policy (W4).
Public API changes require a separate contract review across C, Lua and Rust.
Offline caching, remote streaming, dynamic WASM modules, threads and WebGPU have
no assigned release and remain outside this proposal's acceptance scope.
