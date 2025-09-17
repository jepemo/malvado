# Malvado — Migration Plan to raylib + Lua (mAcceptance criteria

- A1. `make check` passes (luacheck, stylua –or verification–, tests).
- A2. `make run` opens a window and renders the text "Hello, Malvado!".
- A3. No example nor module outside `malvado/backends/*` uses `require("raylib")`.
- A4. `make pack` generates a valid rockspec and tagging `vX.Y.Z` publishes to LuaRocks.
- A5. A mock backend can be injected and tests run without opening a window.
- A6. All functionality works correctly on both macOS and Linux platforms.rst)

Guidance document to execute the migration and clean‑up of Malvado’s API in an orderly way, without duplication, and with clear acceptance criteria.

Status: Approved plan (v1)
Owners: you + support agent (this document is self‑contained)

---

## 1) Goal and scope

Primary goal

- Reimplement/port Malvado on top of raylib via a Lua binding, without C changes, exposing a clean public API under `malvado`, optional global shim, reproducible builds on macOS, and publishing to LuaRocks with basic CI.

Scope (included)

- Backend abstraction: hide raylib behind `malvado` with a minimal contract and backend injection (for tests).
- Modular public API: `init`, `close`, `begin_draw`, `end_draw`, `clear`, `draw_text`, `colors`, `key/is_key_pressed`, etc.
- Compatibility (legacy) layer and optional global shim.
- Tooling: Makefile, luacheck, stylua, busted; minimal scripts; rockspec template.
- CI: lint + tests; release pipeline triggered by tag.

Out of scope (for now)

- Audio, advanced assets/pipeline, hot reload, multiple backends beyond raylib.

Non‑functional

- macOS primary target with full Linux support.
- No C changes (prefer Lua and existing bindings).
- Cross-platform compatibility for major Linux distributions (Ubuntu, Fedora).

---

## 2) Deliverables and acceptance criteria

Deliverables

- E1. `malvado/` modules with hidden raylib backend and minimal contract.
- E2. Updated examples that do not require `raylib` directly.
- E3. Headless tests with a simulated (mock) backend and busted.
- E4. Tooling (Makefile, luacheck, stylua, rockspec) and minimal scripts.
- E5. CI (lint+test) and release pipeline to LuaRocks triggered by tag.

Acceptance criteria

- A1. `make check` passes (luacheck, stylua –or verification–, tests).
- A2. `make run` opens a window and renders the text “Hello, Malvado!”.
- A3. No example nor module outside `malvado/backends/*` uses `require("raylib")`.
- A4. `make pack` generates a valid rockspec and tagging `vX.Y.Z` publishes to LuaRocks.
- A5. A mock backend can be injected and tests run without opening a window.

---

## 3) Phased roadmap (step by step)

### Phase A — Project preparation

1. Repository structure (without moving what already works):

- Add (if missing): `malvado/backends/`, `malvado/compat/`, `tests/`, `scripts/`, `.github/workflows/`, `rockspec/`, `dist/`.
- Keep current modules under `malvado/` and plan their progressive adaptation.

2. Toolchain (macOS):

- Homebrew: cmake, pkg-config, raylib, lua or luajit, luarocks, git, stylua.
- Lua tooling: `luarocks install busted luacheck`.

2. Toolchain (Linux):

- Package managers: cmake, pkg-config, raylib-dev, lua5.4/luajit, luarocks, git, cargo (for stylua)
- Distribution-specific: Ubuntu (`apt`), Fedora (`dnf`), Arch (`pacman`)
- Lua tooling: `luarocks install busted luacheck`.

3. Minimal quality setup:

- Add `.luacheckrc` and `stylua.toml` (consistent style).
- Add `Makefile` with targets: `bootstrap`, `deps`, `check`, `run`, `examples`, `rockspec`, `pack`.

Phase A deliverables: initial Makefile, tooling configs, folders created.  
Phase A acceptance: `make check` runs luacheck and stylua (even if there are no tests yet).

---

### Phase B — Backend contract and raylib adapter

4. Define the backend’s minimal contract (documented in comments):

- Window/loop: InitWindow, CloseWindow, WindowShouldClose, BeginDrawing, EndDrawing, SetTargetFPS.
- Basic rendering: ClearBackground, DrawText, COLORS (WHITE, BLACK… enough for examples).
- Basic input: KEY (e.g., SPACE, LEFT, RIGHT) and IsKeyPressed.
- Utilities: GetFrameTime.

5. Implement `malvado/backends/raylib.lua` mapping the contract to raylib.
6. Add a simulated backend `tests/mocks/backend_mock.lua` that fulfills the contract (no window).

Phase B deliverables: contract (doc + Lua table), raylib backend, mock backend.  
Phase B acceptance: minimal test that requires `malvado`, injects mock, and runs a “frame” without error.

---

### Phase C — Public API and high‑level modules

7. Implement `malvado/init.lua`:

- `use_backend(b)` for injection.
- `init(w,h,title,fps)`, `close()`, `begin_draw()`, `end_draw()`, `clear(color)`, `draw_text(...)`.
- Convenience helpers: `colors()`, `key()`, `is_key_pressed(k)`.

8. Implement specialized modules that receive the backend (via `__use_backend`):

- `malvado/window.lua`: init, set_fps, should_close, close.
- `malvado/graphics.lua`: begin_draw, end_draw, clear, draw_text, colors.
- `malvado/input.lua`: key, is_key_pressed.

9. Optional global shim `malvado/global.lua` that exports the API to `_G` (opt‑in).

Phase C deliverables: `init.lua`, `window.lua`, `graphics.lua`, `input.lua`, `global.lua`.  
Phase C acceptance: a minimal example that only uses `require("malvado")` works.

---

### Phase D — Compatibility (legacy)

10. Layer `malvado/compat/legacy.lua` mapping old names/DSL to the new API.
11. Document the mappings table (what is deprecated and when) and add `Legacy.import_globals()` if needed for older projects.

Phase D deliverables: `legacy.lua` file and mappings document.  
Phase D acceptance: an old example works with `require("malvado/compat/legacy")` or with `Legacy.import_globals()`.

---

### Phase E — Examples and clean‑up

12. Update examples so they never do `require("raylib")`; they must only require `malvado`.
13. Add canonical `examples/hello.lua` with text and background.
14. Review helper duplication (sprite/scene…) and keep them out of the minimal scope; move to optional helpers if retained.

Phase E deliverables: clean and consistent examples.  
Phase E acceptance: `make examples` runs examples without any direct raylib reference.

---

### Phase F — Tests and quality

15. Tests using busted:

- `tests/test_init_spec.lua`: exports basic functions.
- `tests/test_headless_spec.lua`: uses a mock backend and validates begin/clear/end cycle without errors.

16. Lint and formatting: configure `make check` to run luacheck and stylua (verification or controlled auto-formatting).

Phase F deliverables: minimal test suite + Makefile commands.  
Phase F acceptance: `make check` is green locally and in CI.

---

### Phase G — Packaging and publishing

17. Rockspec template at `rockspec/malvado-X.Y.Z-1.rockspec` with version substitution.
18. Makefile targets: `rockspec`, `pack`, `publish` (the latter only in CI or with API key set).
19. Versioning instructions (tags `vX.Y.Z`).

Phase G deliverables: rockspec and functional packaging.  
Phase G acceptance: `make pack` generates the .rock; tag triggers release.

---

### Phase H — CI/CD

20. Workflow `.github/workflows/ci.yml` with: checkout, lua/luarocks, install luacheck/busted, `luacheck .`, `busted -v`.
21. Workflow `.github/workflows/release.yml` that, upon pushing a tag `v*.*.*`, generates rockspec and executes `luarocks upload` with `LUAROCKS_API_KEY`.

Phase H deliverables: active pipelines.  
Phase H acceptance: PR runs CI; tag publishes to LuaRocks.

---

## 4) Target structure (reference)

```
malvado/
  init.lua               # Entry point and orchestration
  global.lua             # Optional shim (_G)
  backends/
  raylib.lua           # Raylib adapter (only binding visible internally)
  compat/
    legacy.lua           # Old API -> new API mappings
  window.lua             # Window/loop API
  graphics.lua           # Basic drawing API
  input.lua              # Input API
tests/
  test_init_spec.lua
  test_headless_spec.lua
  mocks/backend_mock.lua
scripts/
  build_raylib_macos.sh  # Optional, if not using Homebrew
  build_binding_macos.sh  # Optional, if not using Homebrew
rockspec/
  malvado-X.Y.Z-1.rockspec
.github/workflows/
  ci.yml
  release.yml
```

Note: use Homebrew for raylib if you prefer simplicity; use the scripts only if you need full determinism.

---

## 5) Backend contract (summary)

Minimal functions (suggested names):

- Window/loop: `InitWindow(w,h,title)`, `CloseWindow()`, `WindowShouldClose() -> bool`, `BeginDrawing()`, `EndDrawing()`, `SetTargetFPS(n)`.
- Rendering: `ClearBackground(color)`, `DrawText(str,x,y,size,color)`, `COLORS = { WHITE, BLACK, ... }`.
- Input: `KEY = { SPACE, LEFT, RIGHT, ... }`, `IsKeyPressed(key)`.
- Utilities: `GetFrameTime() -> number`.

Golden rule: only grow the contract when a real use‑case justifies it; keep it small.

---

## 6) Makefile (minimal targets)

- `bootstrap`: installs tools (Homebrew/LuaRocks) — optional.
- `deps`: installs dependencies (raylib via Homebrew or scripts).
- `examples` and `run`: run examples with `lua -l malvado`.
- `test`, `lint`, `fmt`, `check`: quality.
- `rockspec`, `pack`, `publish`: packaging and publishing.

You can reuse the existing repository content and adapt it to these targets.

---

## 7) Risks and mitigations

- Differences between Lua 5.4 and LuaJIT: pin version in CI and in docs; test both if possible.
- Binding instability: pin raylib and binding versions/tags; vendor scripts if needed.
- Large legacy API: prioritize most used mappings; add deprecation warnings and a removal timeline.

---

## 8) Plan improvements and recommendations

### Enhanced Testing Strategy

- Add migration validation tests that verify identical behavior between old and new implementations
- Include performance comparison benchmarks between current and raylib versions
- Create stress tests for potential performance bottlenecks
- Expand Phase F to include compatibility verification tests

### Documentation and User Transition

- Add Phase I (Documentation): API reference updates, migration guides, updated examples documentation
- Create user migration guide explaining how to transition existing code
- Consider dual-backend support during transition period (if technically feasible)
- Provide clear version signaling for compatibility

### Technical Debt and Code Quality

- Use migration opportunity to address known issues in current codebase
- Review and standardize error handling patterns
- Improve code documentation and type annotations
- Standardize module interfaces and patterns

### Enhanced Risk Management

- Document specific technical risks for each module during implementation
- Create contingency plans for critical compatibility issues
- Define rollback strategy if major issues are discovered post-release
- Add pre-implementation compatibility assessment phase

### Timeline Adjustments

- Add initial compatibility assessment (1 week before Phase A)
- Include documentation updates concurrent with Phases C-H
- Plan community feedback period before final release
- Buffer time for unexpected compatibility challenges

### Linux Support Enhancements

- **Toolchain expansion for Linux**: Document package installation for major distributions (apt/dnf: cmake, pkg-config, raylib-dev, lua5.4/luajit, luarocks, git, cargo for stylua)
- **Cross-platform detection**: Add `detect_platform()` function in Phase B to handle platform-specific differences
- **Script updates**: Rename `build_raylib_macos.sh` → `build_raylib.sh` with platform detection; add `scripts/setup_linux_deps.sh`
- **CI matrix expansion**: Include both macOS and Linux in CI pipeline with multiple Lua versions (5.1, 5.4, luajit)
- **Path handling**: Document and handle different installation paths between platforms (/usr/local vs /usr)
- **Additional risks**: Platform-specific performance differences, input behavior inconsistencies, resource path variations

---

## 9) Version roadmap

### v0.1.0-alpha — Foundation

- **Phases A-B**: Project structure, tooling setup, backend contract
- **Deliverables**:
  - Basic Makefile with essential targets
  - Backend abstraction layer with raylib adapter
  - Mock backend for headless testing
  - Initial CI setup (lint + basic tests)
- **Milestone**: Core architecture established, tests pass headlessly

### v0.1.0-beta — Core API

- **Phases C-E**: Public API implementation, examples migration
- **Deliverables**:
  - Complete `malvado/` module API (`init`, `window`, `graphics`, `input`)
  - Updated examples using only `malvado` (no direct raylib)
  - Legacy compatibility layer for existing code
  - Cross-platform support (macOS + Linux)
- **Milestone**: Public API stable, examples work on both platforms

### v0.1.0-rc — Quality & Documentation

- **Phases F-H**: Testing, packaging, release preparation
- **Deliverables**:
  - Comprehensive test suite with CI/CD
  - Complete documentation and migration guides
  - LuaRocks packaging and publication pipeline
  - Performance validation and benchmarks
- **Milestone**: Production-ready release candidate

### v0.1.0 — Public Release

- **Final validation**: All acceptance criteria met
- **Community release**: Published to LuaRocks, documentation complete
- **Support**: Migration guides, compatibility guarantees established

### Post v0.1.0 — Future Versions

- **v0.2.x**: Enhanced features, additional backends, hot reload
- **v1.0.x**: API stability guarantees, deprecated legacy layer removal

---

## 10) Final checklist (Go/No‑Go)

- [ ] No `require("raylib")` outside `malvado/backends/*`.
- [ ] `make check` is green locally and in CI.
- [ ] Hello example runs with `require("malvado")` and shows text.
- [ ] Mock backend enables headless tests.
- [ ] Valid rockspec; `make pack` generates correct artifact.
- [ ] Tag `vX.Y.Z` publishes to LuaRocks from CI.

---

## 11) Next steps (post v0.1)

- Complete legacy compatibility mappings with coverage of real examples.
- Backend capabilities system (detect feature support).
- Lua‑only hot reload for development.
- Document public API in `README.md` (without mentioning bindings; internal detail).

```

```
