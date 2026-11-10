# Rust native SDK

This workspace provides `malvado`, the safe owner-thread async process API,
and `malvado-sys`, the unsafe native boundary. Graphics and platform resource wrappers remain in the [alpha.7 checklist](../../docs/versions/2.0.0-alpha.7.md).

Use the project toolchain and run all local SDK checks:

```sh
mise install
mise run test-rust
```

`mise` pins Rust, CMake and Python, including the required Clippy and rustfmt components. The task builds C-only Debug and Release
SDKs without Cargo on their native build PATH, installs and relocates each SDK
to a path with spaces, then runs Rust tests, clippy and formatting. It also
checks missing, incomplete, incompatible, and dynamic-only SDKs and builds an
independent offline consumer from the installed crate sources. No native source-tree
paths are needed by that consumer. The same task runs in macOS/Linux CI.

For an independent Cargo project, depend on `<prefix>/share/malvado/rust/malvado`
by path and set `MALVADO_SDK_PREFIX` to your installed native SDK:

```sh
MALVADO_SDK_PREFIX="/path/to/installed sdk" cargo test
```

## Unsafe native boundary

`malvado-sys` declares context, cadence, process identity/lifecycle and
resource operations. See the installed `malvado/malvado.h` for pointer,
ownership, errors and thread rules. Raw calls are unsafe. Call `check_abi()`
before other native operations. Return values are C integers, not Rust enums;
unknown values must not be transmuted into Rust enum variants. Callback panics
must never unwind across C. `malvado-sys` does not catch them for its callers.

## Native discovery contract

The prefix is required. Cargo does not search the checkout, build the engine,
download sources, install tools, or fall back to a system library. `build.rs`
uses CMake to find only the installed package in the prefix. A small C probe
checks every declared native signature against its header and reports field
sizes, alignments, offsets, and enum values. Rust checks these at compile time
for every consumer. Negative fixtures verify signature and layout mismatch rejection. It runs the ABI
query first; a different revision stops validation before the release query.
An exact release match is also required. New incompatible libraries must
increment the ABI revision before changing any other query contract.

The supported product is the existing `libmalvado_core.a` archive with its
math-library dependency. Dynamic-only SDKs are explicitly rejected: Malvado
does not currently build a shared core product. There is no dynamic-loader
path to configure for the static core. The optional graphics feature checks and links its static raylib/backend
archives and supported host dependencies separately.
Cargo's `links` key prevents duplicate sys packages for the same native engine.

The host C compiler must match the Rust target. The executable probe detects
wrong native architectures through link or execution errors. Cross compilation,
Windows and Android are not supported by this initial SDK. A CMake package
which resolves the core archive outside the selected prefix is rejected.
SDK prefixes cannot contain newlines, carriage returns or semicolons.

No crates.io package, complete graphics API, player export, or completed alpha.7
release is implied. The development engine and crate identifiers now read
`2.0.0-alpha.7`; the milestone is still in progress.

## Scheduler proof checks

After `mise run test-rust` installs a local native SDK:

```sh
mise install --cd tools/miri rust
mise run test-rust-miri
mise run test-rust-sanitize
```

The optional dated nightly provides Miri without changing the stable root
toolchain. Miri checks pure future/waker ownership. The sanitizer task requires
Clang (and lld on Linux) and checks native C callbacks with ASan/UBSan; Rust itself is not sanitizer
instrumented. See the [proof and limitations](../../docs/versions/2.0.0-alpha.7-scheduler.md).

For safe game processes, start with the [Rust process guide](../../docs/api/rust-processes.md)
and `malvado/examples/counter.rs`. The unsafe sys crate is not an alternate
recommended gameplay style.

The [native collision/world guide](../../docs/api/rust-collision.md) covers
explicit geometry and process colliders. It does not promise graph-derived
colliders or contextual process movement. `malvado/examples/world.rs` is a
bounded headless example.

The [camera and animation guide](../../docs/api/rust-camera-animation.md) covers
native camera values and explicit fixed-tick playback without a graphics
backend. Automatic process camera selection and graph animation remain pending.

Enable the optional `assets` feature for [native atlas metadata](../../docs/api/rust-atlas.md).
The installed C SDK must include assets. Verify it with `mise run test-rust-assets`.
Default features still link only the core and need no JSON library.

Enable `graphics` for the [owned window service](../../docs/api/rust-window.md).
The SDK must contain static graphics and raylib libraries.
Run `mise run test-rust-graphics` with a real or virtual display.

The [drawing and image guide](../../docs/api/rust-drawing.md) covers immediate
shapes/text, scoped clip/scroll, owned PNG/RGBA images and queued-frame retention.
These services do not assign process graphs or collect process draws yet.

The [FPG guide](../../docs/api/rust-fpg.md) covers the accepted uncompressed
BennuGD 8-bit subset, indexed data, conversion and metadata manifests. The
optional graphics bridge creates owned images without keeping FPG data alive.
Full DIV FPG compatibility and other binary variants are not claimed.

The [font guide](../../docs/api/rust-font.md) covers owned TTF/OTF and selected
Unicode glyphs. The [input guide](../../docs/api/rust-input.md) covers typed
mouse/gamepad queries and consuming native Unicode text.
