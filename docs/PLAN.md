
# Malvado ➜ raylib + Lua Migration Plan (macOS-first)

**Status:** Draft v0.1  
**Audience:** You (and an AI agent).  
**Goal:** Reimplement/port **malvado** to **raylib + Lua** (no C changes) with a clean module API, optional global shim, reproducible builds on macOS, and basic CI + LuaRocks publishing.

---

## 0) Design Principles (recap)

- **Single namespace:** Everything lives under `malvado` modules.  
- **Optional globals:** Opt-in `require("malvado/global")` or `malvado.import()` for DIV-style ergonomics.  
- **Helpers on top of primitives:** Keep helpers (`sprite`, `scene`, etc.) separate from raw raylib calls.  
- **Testability:** Functions are methods on tables you can stub in tests.  
- **Portability:** Prefer Lua-side glue; avoid C changes unless strictly needed.

---

## 1) Repository Layout

```
malvado-raylib/
├─ malvado/                      # Lua sources (the engine surface)
│  ├─ init.lua                   # public API & import()
│  ├─ global.lua                 # optional global shim
│  ├─ sprite.lua                 # helpers
│  ├─ scene.lua                  # helpers
│  └─ util/                      # internal utilities (timers, easing, …)
│     ├─ timers.lua
│     └─ easing.lua
├─ examples/
│  ├─ hello.lua
│  └─ sprites.lua
├─ tests/
│  ├─ test_init_spec.lua
│  └─ test_sprite_spec.lua
├─ scripts/
│  ├─ build_raylib_macos.sh      # fetch & build raylib (desktop, release)
│  ├─ build_binding_macos.sh     # fetch & build Lua binding to raylib
│  ├─ package.sh                 # package rock & artifacts
│  └─ version.sh                 # echoes project version from tag or file
├─ vendor/                       # optional: pinned source checkouts
│  ├─ raylib/
│  └─ raylib-lua-binding/
├─ dist/                         # produced artifacts (rocks, zips, …)
├─ rockspec/
│  └─ malvado-X.Y.Z-1.rockspec   # template; CI fills version
├─ .github/
│  └─ workflows/
│     ├─ ci.yml
│     └─ release.yml
├─ .editorconfig
├─ .gitignore
├─ .luacheckrc
├─ stylua.toml
├─ Makefile
└─ README.md
```

---

## 2) Tools & Dependencies (macOS)

### Preferred (fast path)
- **Homebrew:** `brew install cmake pkg-config raylib luajit lua luarocks git`
- **Lua tooling:** `luarocks install busted luacheck` and install `stylua` via Homebrew: `brew install stylua`

### Deterministic (from source)
- `scripts/build_raylib_macos.sh` will clone & build raylib into `vendor/raylib/build` and generate `.dylib`/`.a` + headers.
- `scripts/build_binding_macos.sh` will clone & build your selected **Lua ↔ raylib** binding into `vendor/raylib-lua-binding`.

---

## 3) Choose a Binding

Set in **Makefile**:
- `BINDING ?= raylib-lua` (LuaJIT-friendly, usually the smoothest path)
- or `BINDING ?= raylib-lua-sol` (Lua 5.4 + sol2, header-only C++)

---

## 4) Makefile (single entry point)

```makefile
# Makefile — macOS-first; Linux likely works with minimal changes
# --------------------------------------------------------------
SHELL := /bin/bash

PROJECT_NAME      ?= malvado
VERSION           ?= $(shell scripts/version.sh)
RAYLIB_TAG        ?= 5.x
BINDING           ?= raylib-lua
BINDING_TAG       ?= latest
USE_HOMEBREW      ?= 1
LUAROCKS_API_KEY  ?=

ROOT    := $(CURDIR)
VENDOR  := $(ROOT)/vendor
DIST    := $(ROOT)/dist
SCRIPTS := $(ROOT)/scripts

LUA     ?= lua
LUAROCKS ?= luarocks

.PHONY: all bootstrap deps raylib binding build run examples test lint fmt check rockspec pack publish clean distclean

all: check build

bootstrap:
	@if [ "$(USE_HOMEBREW)" = "1" ]; then \
	  brew update; \
	  brew install cmake pkg-config raylib lua luajit luarocks stylua || true; \
	fi
	@$(LUAROCKS) install busted || true
	@$(LUAROCKS) install luacheck || true

deps: bootstrap raylib binding

raylib:
	@if [ "$(USE_HOMEBREW)" = "1" ]; then \
	  echo "› Using Homebrew raylib"; \
	else \
	  bash $(SCRIPTS)/build_raylib_macos.sh $(RAYLIB_TAG); \
	fi

binding:
	@bash $(SCRIPTS)/build_binding_macos.sh $(BINDING) $(BINDING_TAG)

build:
	@find malvado -name '*.lua' -print0 | xargs -0 -n1 $(LUA) -l || true

run:
	@$(LUA) -l malvado examples/hello.lua

examples:
	@for ex in examples/*.lua; do $(LUA) -l malvado $$ex; done

test:
	@busted -v

lint:
	@luacheck .

fmt:
	@stylua --verify . || stylua .

check: lint fmt test

rockspec:
	@mkdir -p $(DIST)
	@sed -e "s/@VERSION@/$(VERSION)/g" rockspec/malvado-X.Y.Z-1.rockspec > $(DIST)/$(PROJECT_NAME)-$(VERSION)-1.rockspec

pack: rockspec
	@cd $(ROOT) && $(LUAROCKS) pack $(DIST)/$(PROJECT_NAME)-$(VERSION)-1.rockspec

publish: pack
	@test -n "$(LUAROCKS_API_KEY)"
	@$(LUAROCKS) upload
```

---

## 5) Scripts

**scripts/build_raylib_macos.sh**  
```bash
#!/usr/bin/env bash
set -euo pipefail

TAG="${1:-5.x}"
ROOT="$(cd "$(dirname "$0")/.."; pwd)"
VENDOR="$ROOT/vendor"
RAYLIB_DIR="$VENDOR/raylib"

mkdir -p "$VENDOR"
if [ ! -d "$RAYLIB_DIR" ]; then
  git clone --depth=1 --branch "$TAG" https://github.com/raysan5/raylib.git "$RAYLIB_DIR"
fi

cd "$RAYLIB_DIR"
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON ..
cmake --build . --config Release --parallel
```

**scripts/build_binding_macos.sh**  
```bash
#!/usr/bin/env bash
set -euo pipefail

BINDING="${1:-raylib-lua}"
TAG="${2:-latest}"
ROOT="$(cd "$(dirname "$0")/.."; pwd)"
VENDOR="$ROOT/vendor"
BIND_DIR="$VENDOR/raylib-lua-binding"

mkdir -p "$VENDOR"
if [ ! -d "$BIND_DIR" ]; then
  git clone --depth=1 https://github.com/TSUNDERE/raylib-lua "$BIND_DIR" || true
fi

pushd "$BIND_DIR" >/dev/null

case "$BINDING" in
  raylib-lua)
    make -C src || true
    ;;
  raylib-lua-sol)
    mkdir -p build && cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    cmake --build . --config Release --parallel
    ;;
esac

popd >/dev/null
```

---

## 6) Examples

**examples/hello.lua**  
```lua
local rl = require("raylib")
local M  = require("malvado")

M.init(800, 450, "malvado + raylib (hello)")
rl.SetTargetFPS(60)

while not rl.WindowShouldClose() do
  M.begin_draw()
    M.clear(rl.RAYWHITE)
    M.draw_text("Hello, Malvado!", 20, 20, 24, rl.BLACK)
  M.end_draw()
end

M.close()
```

---

## 7) Testing

**tests/test_init_spec.lua**  
```lua
describe("malvado.init", function()
  it("exports init/close/draw", function()
    local M = require("malvado")
    assert.is_function(M.init)
    assert.is_function(M.close)
    assert.is_function(M.begin_draw)
  end)
end)
```

---

## 8) CI Pipelines

**.github/workflows/ci.yml**  
```yaml
name: CI
on: [push, pull_request]
jobs:
  lint_test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - uses: leafo/gh-actions-lua@v10
        with:
          luaVersion: "5.4"
      - uses: leafo/gh-actions-luarocks@v4
      - run: luarocks install busted luacheck
      - run: luacheck .
      - run: busted -v
```

**.github/workflows/release.yml**  
```yaml
name: Release
on:
  push:
    tags: ["v*.*.*"]
jobs:
  publish-luarocks:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - uses: leafo/gh-actions-luarocks@v4
      - run: |
          TAG="${GITHUB_REF_NAME#v}"
          mkdir -p dist
          sed -e "s/@VERSION@/${TAG}/g" rockspec/malvado-X.Y.Z-1.rockspec > dist/malvado-${TAG}-1.rockspec
          luarocks upload --api-key="${LUAROCKS_API_KEY}" dist/malvado-${TAG}-1.rockspec
        env:
          LUAROCKS_API_KEY: ${{ secrets.LUAROCKS_API_KEY }}
```

---

## 9) Acceptance Criteria

- `make deps && make run` opens a window and prints text.  
- `make check` passes (lint, format verify, tests).  
- `make pack` produces a valid rockspec.  
- Tagging `vX.Y.Z` triggers CI release and publishes to LuaRocks.  

---

*Happy hacking!*
