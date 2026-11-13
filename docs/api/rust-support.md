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

CLI project creation, build/check/run and exports are delivered for matching
native macOS/Linux and Windows x64 MSVC SDKs. Windows headless/graphical SDKs,
CLI authoring and fresh exported players are verified in the recorded baseline.
Windows GNU/ARM64,
Rust cross compilation and Android Rust game paths are not delivered. Software
rendering and silent audio CI do not establish physical device behavior. See
[platform verification](../development/alpha8-platforms.md) for evidence and
limits, and the [language matrix](language-parity.md) for feature differences.

## Current entry point

Use `Game::new()` and the existing owned async process API. The constructor
creates the same native context as the former `Engine::new()`: a 60 Hz clock
with a five-step advance limit. It does not create a window, run processes,
poll input or start an application loop. See [Rust processes](rust-processes.md)
and the [rename migration](../development/rust-game-migration.md).

Ordinary structs with owned `async fn run(self, ProcessContext)` remain the
baseline. Macros, direct native property fields, shorter spawning and higher
level initialization are [future proposals](../roadmap/rust-ergonomics.md).
Their sample syntax is not available in this SDK and is not required for 2.0.
