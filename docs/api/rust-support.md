# Experimental Rust support

Malvado 2.0 supports C, Lua and experimental Rust. The experimental label applies
to the Rust gameplay SDK and its public API. It does not label the C core or the
whole engine as experimental.

The Rust SDK is functional, maintained and tested. It provides native async game
processes over the same C engine and scheduler as C and Lua. External Cargo
projects can use the installed SDK. Supported features follow their documented
contracts, including ownership, cancellation, errors and cleanup.

The public API's ergonomics, naming and process syntax are not final. Future
2.x releases may introduce breaking Rust API changes before Rust support is
declared stable. Significant changes need migration guidance. There is no
long-term Rust API stability guarantee. The existing
[development-version policy](compatibility.md) still applies; experimental
status does not permit silent changes to contracts or native ABI checks.

## Development and players

Developers need an installed Rust toolchain, CMake, a C compiler and a matching
native Malvado SDK. Rust games compile to native executables. Players do not
need Cargo or rustc. Runtime libraries, assets and notices must still accompany
an exported game according to its target's distribution contract.

Cargo rechecks the native SDK when `CMAKE_GENERATOR`,
`CMAKE_GENERATOR_PLATFORM`, `CMAKE_GENERATOR_TOOLSET` or
`CMAKE_GENERATOR_INSTANCE` changes. An invalid selection must fail native probe
configuration even after a successful build. Restore the valid selection to
rebuild the probe; the generated probe cache is not reused across changes.
CMake applies its platform/toolset/instance environment defaults only when
`CMAKE_GENERATOR` is explicit. The shared Windows platform verifier sets all
four selectors. No gameplay API or native ABI changes are required.

CLI project creation, build/check/run and exports are delivered for matching
native macOS/Linux and Windows x64 MSVC SDKs. Windows headless/graphical SDKs,
CLI authoring and fresh exported players are verified in the recorded baseline.
The installed CLI exports Rust games as Android arm64 debug APKs and release
APKs or AABs from supported macOS and Linux hosts. These builds use NDK r27c
and target-side ABI probes. The alpha.8 baseline ran APKs and APKs derived
from AABs on API 34, including Activity lifecycle, input, audio focus and IME
checks. The x86_64 emulator fixture path is for verification. The delivered
export target is arm64. See the [Android Rust commands](../development/android-rust-games.md).
Physical device behavior and audible output remain beta checks. See
[platform verification](../development/platforms.md) for evidence and
limits, and the [language matrix](language-parity.md) for feature differences.

## Current entry points

For windowed games, create a `Game::windowed`, spawn named async functions,
and call `run`. This uses the native 60 Hz logical clock, window services,
and scheduler. `Game::new()` remains available for headless use or manual
hosting. See [Rust processes](rust-processes.md) and the
[Rust migration guide](../development/rust-game-migration.md).

Owned state structs with an `async fn run(self, ProcessContext)` also work with
the same spawn API. Macro declarations and typed collision queries remain
[open design topics](../roadmap/rust-ergonomics.md).
