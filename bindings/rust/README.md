# Rust native SDK

Rust support is experimental in Malvado 2.0. The gameplay SDK is functional,
maintained and tested over the shared native C scheduler. Its public API can
change in future 2.x releases before Rust support is declared stable. Significant
changes include migration guidance; no long-term API stability is promised.
See the [support policy](../../docs/api/rust-support.md) for platform and toolchain limits.

This workspace provides `malvado`, the safe owner-thread async process API,
and `malvado-sys`, the unsafe native boundary. Optional `graphics` and `assets`
features wrap the delivered native platform and asset services. Start an
installed project with `malvado new --lang rust my-game`; see the
[CLI guide](../../docs/specs/cli.md#rust-projects) and
[language matrix](../../docs/api/language-parity.md).

Use `Game::new()` as the main Rust entry point. It owns the native context;
it does not open a window or run a host loop. Keep ordinary owned async process
methods and explicit platform services. Existing projects can follow the
[Engine to Game migration](../../docs/development/rust-game-migration.md).

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

The supported core is a static archive: `libmalvado_core.a` on Unix and
`malvado_core.lib` on Windows. Native probes check the host dependencies. Dynamic-only SDKs are explicitly rejected: Malvado
does not currently build a shared core product. There is no dynamic-loader
path to configure for the static core. The optional graphics feature checks and links its static raylib/backend
archives and supported host dependencies separately.
Cargo's `links` key prevents duplicate sys packages for the same native engine.

The host C compiler must match the Rust target. The executable probe detects
wrong native architectures through link or execution errors. Cross compilation and Android are not supported. Native Windows x64 MSVC
core and assets validation is available through `mise run test-platform -- --rust`;
Use `mise run test-platform -- --graphics --rust --work build-platform-graphics`
to verify the graphical SDK separately. Windows x64 MSVC CLI builds and folder
exports are verified; see the [platform matrix](../../docs/development/alpha8-platforms.md). A CMake package
which resolves the core archive outside the selected prefix is rejected.
SDK prefixes cannot contain newlines, carriage returns, semicolons or quotation marks.

The installed CLI supports native Rust folder exports and macOS app bundles.
No crates.io package or cross-compilation support is implied. The current
native engine and crate identifiers are `2.0.0-alpha.7`; development toward 2.0
continues under the [version plan](../../docs/versions/README.md). This identifier
alone does not certify platform support or Rust API stability.

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

The [audio guide](../../docs/api/rust-audio.md) covers the independent audio
device, owned WAV instances, independent clones, WAV/OGG streams and explicit playback maintenance.
Audio currently uses the optional `graphics` platform feature.

The [fade guide](../../docs/api/fades.md) covers the shared native FIFO and
copied Rust values with explicit tick advancement.

[Simple graphic geometry](../../docs/api/graphic-geometry.md) supplies native
rectangle/circle bounds and pivots without the graphics feature.

[Drawing defaults](../../docs/api/draw-state.md) are copied native values used
as explicit background and color arguments. They install no global Rust state.

[Process visual order](../../docs/api/process-visual-order.md) covers native
process depth and ordered visible handles without a graphics dependency.

## Verification boundary

Keep `malvado` library unit tests independent of C FFI. The optional
`mise run test-rust-miri` task executes that library target to verify Rust future,
waker, pinned state and query-buffer ownership. Miri cannot execute native C.

Tests that create an engine, owned native value or platform resource belong in
integration tests. `tests/text_values.rs` covers native text copies, atomic
updates and alignment boundaries through the public safe API. The normal SDK
and sanitizer tasks execute these tests against the real native library and
retain native ABI checks.
