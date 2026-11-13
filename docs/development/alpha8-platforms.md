# Alpha.8 platform verification

Alpha.8 is in progress. A configured CI job is not proof of platform support.
Use the logs from the exact candidate revision to establish support.

| Host / target | Current verification | Remaining work |
| --- | --- | --- |
| macOS / native | C/Lua/Rust and graphics in existing CI | Keep checks passing |
| Ubuntu 24.04 / native | C/Lua/Rust, graphics and clean exports in existing CI | Keep checks passing |
| Windows Server 2022 / x64 MSVC | New headless C/Lua SDK job; native results required | Native Rust results, graphics/input/audio, CLI builds and exports |
| Desktop / Android | Not implemented | C/Lua/Rust runtime, APK/AAB and emulator behavior |

## Native Windows baseline

Use the Visual Studio 2022 C++ desktop workload and Windows SDK on x64.
CMake uses its Visual Studio generator and `--config` for each build. CMake,
Python and Rust versions come from the root `mise.toml`. The `windows-2022`
runner image can update MSVC and the Windows SDK. Each run records the exact
installed versions; this is not a hermetic compiler image.

Run `mise run test-platform` locally or in CI. It builds Debug and Release
headless SDKs with bundled Lua and atlas services. The CTest suite tests core
behavior, failure paths, process lifetime, mixed C/Lua ownership, CLI operations
that do not need graphics, and installed C SDK consumers. Every command has a
bounded timeout. Logs remain in `build-platform`, including on failure.
The verifier also installs each SDK and moves it to a path with spaces.

The Windows workflow runs on pushes, pull requests, manual requests and weekly.
It uploads command and CTest logs even when a step fails. The existing Linux and
macOS workflow remains required. Candidate acceptance must also check Windows;
the existing SDK candidate collector does not yet include Windows packages.

Windows GUI, input and audible output are not established by headless tests.
Android toolchain and emulator selection remain part of A8.1; do not infer
Android support from desktop or Rust native success. Android is an export target,
not an Android CLI authoring host. No store credentials are needed for these tests.

## Rust native SDK path

Use `mise run test-platform -- --rust` to run the safe Rust workspace tests with
core and optional assets against each relocated SDK. The Windows job uses this
command. Windows support is limited to native `x86_64-pc-windows-msvc`; MinGW,
Windows ARM64, cross compilation and Rust graphics are rejected. The SDK probe
still validates ABI values, layouts and signatures before linking. MSVC uses
`.lib` archives, `.exe` probes and BCrypt instead of the Unix math library.
Native Windows results remain required before closing A8.2.

The Rust verifier also builds and runs an offline consumer from the installed
crate sources in a project path with spaces. The consumer checks process
execution and cleanup. Both configurations must reject missing and nonexistent
SDK prefixes with the documented errors. These diagnostics use Cargo and CMake;
they do not depend on Unix archive tools.

MSVC SDK targets use the DLL release CRT (`/MD`) in both Debug and Release.
This matches Rust's native runtime. Debug builds retain symbols and assertions.
A Debug CRT SDK cannot be linked into the default Rust runtime: Windows run
[43412891860](https://github.com/jepemo/malvado/actions/runs/43412891860) exposed
unresolved debug-CRT atomic helper symbols after passing all 202 C/Lua checks.
