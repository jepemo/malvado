# Alpha.8 platform verification

Alpha.8 is in progress. A configured CI job is not proof of platform support.
Use the logs from the exact candidate revision to establish support.
Malvado 2.0 supports C, Lua and experimental Rust. The experimental label applies
to the maintained Rust gameplay API, not the C core. It does not widen the
platform claims below; see the [Rust support policy](../api/rust-support.md).

| Host / target | Current verification | Remaining work |
| --- | --- | --- |
| macOS / native | C/Lua/Rust and graphics in existing CI | Keep checks passing |
| Ubuntu 24.04 / native | C/Lua/Rust, graphics and clean exports in existing CI | Keep checks passing |
| Windows Server 2022 / x64 MSVC | Verified headless and Mesa graphics C/Lua/Rust SDKs in Debug and Release | CLI builds and exports; hardware review in beta |
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
command. Windows support is limited to native `x86_64-pc-windows-msvc`.
Windows GNU targets, Windows ARM64 and cross compilation are rejected. The SDK probe
still validates ABI values, layouts and signatures before linking. MSVC uses
`.lib` archives, `.exe` probes and BCrypt instead of the Unix math library.
Native Windows verification passed on `c49545c` in
[run 43437979697](https://github.com/jepemo/malvado/actions/runs/43437979697).
Both configurations passed headless and graphical Rust tests, installed crate
consumers and SDK diagnostics. Windows CLI builds and exports remain open in A8.3.

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

Asset source paths use UTF-8 on Windows. The core converts them to UTF-16 for
`_wfopen`; C, Lua and Rust use the same file snapshot implementation. Unix byte
paths retain their existing behavior. Rust rejects unpaired UTF-16 surrogates
and embedded NUL characters with `InvalidArgument`. Tests open and remove a
source with accented and Japanese characters, verify image path joins and check
invalid Windows surrogate input. Windows allocation-failure coverage includes
the temporary UTF-16 path allocation.

## Windows graphics verification

`mise run test-platform -- --graphics --rust --work build-platform-graphics`
uses the same Debug/Release verifier with graphics enabled. It builds the
installed C/Lua/Rust SDK, runs five bounded C pixel/window/scheduler checks,
then runs the Rust graphics workspace and an installed-crate window consumer.
The headless matrix entry stays separate. Native verification passed on `c49545c` in
[Windows run 43437979697](https://github.com/jepemo/malvado/actions/runs/43437979697),
closing A8.2 for this supported baseline.

Hosted Windows graphics verification uses Mesa 26.2.6 llvmpipe with the default
OpenGL 3.3 engine and paced silent audio. The MinGW x64 Mesa archive is pinned by
SHA-256 in `tools/windows_graphics.py`. The verifier downloads and verifies it,
then deploys the driver DLLs beside C test and Cargo executables. Cargo uses a
bounded runner wrapper. No system driver is replaced and Mesa is not added to
SDK/game packages. The [Mesa distribution](https://github.com/pal1000/mesa-dist-win)
remains a test-only dependency. This checks the current 2D engine;
it does not establish GPU-driver support or audible output. Physical input and
audio review remain beta work. The default desktop renderer remains unchanged.
Rust image/font/audio wrappers share the existing native path conversion.
Windows media filename encoding beyond the asset snapshot service remains a
verification limitation.

Windows Rust graphics checks the oldest live process thread before opening a
window or audio device. Rust exits the process when main returns. Incomplete
thread inventories and equal oldest creation times return an error; no thread
identity is cached. Every snapshot and thread handle is closed. The native
implementation uses [Thread32Next](https://learn.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-thread32next)
and [GetThreadTimes](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getthreadtimes).
Existing worker-thread rejection and repeated window/audio cleanup tests cover
this mapping. There is no new scheduler or public thread API.

A working WGL provider is required. [Run 43436269922](https://github.com/jepemo/malvado/actions/runs/43436269922) documented that the hosted
Windows system driver cannot create a window; raylib 5.5 aborts in that unsupported
configuration. The Mesa entry validates the supported software-renderer path.
The verifier checks the archive digest before extraction, requires both WGL DLLs
and copies only those two libraries beside executable files. Its host-independent
regression tests cover checksum rejection, incomplete drivers and deployment.

## Windows CLI and exported players

Native verification is pending for this implementation. The graphical matrix
uses `mise run test-platform -- --graphics --rust --cli --work build-platform-graphics`.
For each configuration, `tests/windows_cli.py` creates C/Lua/Rust games in paths
with spaces, checks/builds/runs them, and exports bounded graphical players.
It verifies binary assets, notices, reserved names, no overwrite, missing tools,
SDK diagnostics and exclusion of native source/build outputs. C/Rust children
receive explicit arguments and SDK environment without modifying the CLI host.

A separate Windows job downloads only the exported players, not the producer's
SDK or authoring projects. It runs them from another directory with a PATH that
has no Cargo, rustc, CMake or compiler. Mesa is provisioned only by the test
harness after package checks. The ordinary packages do not contain this driver.
The runtime requires a suitable WGL provider and the Microsoft release CRT;
Debug CRTs are not the default game dependency. Physical systems and audible
output remain beta checks.

Folder exports use `game.exe`; native C/Rust games also contain `game-bin.exe`.
Existing portable reserved names remain reserved, with Windows executable names
and case folding added. Reparse points, including junctions, are rejected in
project entries and managed build directories. Publication never replaces an
existing output, and failed exports remove only their owned staging directory.
Windows `--app` exports are unavailable. The supported authoring generator is
Visual Studio 2022 x64; the Unix Ninja multi-config fixture does not establish a
Windows Ninja toolchain. The Unix managed developer installer (`tools/dev_sdk.py`)
is not a Windows installer. Use an installed SDK prefix and its `bin/malvado.exe`.
