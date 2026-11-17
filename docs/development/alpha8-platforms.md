# Alpha.8 platform verification

Alpha.8 is in progress. A configured CI job is not proof of platform support.
Use the logs from the exact candidate revision to establish support.
Malvado 2.0 supports C, Lua and experimental Rust. The experimental label applies
to the maintained Rust gameplay API, not the C core. It does not widen the
platform claims below; see the [Rust support policy](../api/rust-support.md).

| Host / target | Current verification | Remaining work |
| --- | --- | --- |
| macOS 15 arm64 / native | C/Lua/Rust and graphics in existing CI | Keep checks passing |
| Ubuntu 24.04 x86_64 / native | C/Lua/Rust, graphics and clean exports in existing CI | Keep checks passing |
| Windows Server 2022 / x64 MSVC | Verified headless and Mesa graphics C/Lua/Rust SDKs in Debug and Release | Verified CLI/export and fresh players; hardware review in beta |
| macOS arm64 / Android arm64 API 34 | Headless C/Lua/Rust core and assets, Rust NativeActivity window/assets/errors/cleanup and Back/resume/keyboard/contact/audio, C graphical SDK and both Lua APK entries/assets, Back/resume, primary contact, audio API state/ownership and explicit pause continuity in Debug and Release | Remaining input/lifecycle/audio-focus, general packaged-game integration, CLI APK/AAB and remaining input/audio-focus behavior |
| Ubuntu 24.04 x86_64 / Android x86_64 API 34 | Headless C/Lua/Rust core/assets, C/Rust graphical SDK fixtures and Lua APK, Back/resume, primary contact and audio API/explicit-pause cases with screenshots passed in CI | Remaining input/lifecycle/audio-focus, general packaged-game integration, CLI APK/AAB and remaining input/audio-focus behavior |

Android audio verification uses silent packaged WAV/OGG media. It checks API
state, resource lifetime and explicit game-requested pause across Activity
transitions. It does not establish audible output, latency or automatic audio
focus handling. See [Android audio verification](android.md#android-audio-api-verification).

Android C and both Lua entries also passed physical-key A/Left state and
transition checks on arm64 and in pinned x86_64 CI, in both profiles and after
Activity resume. This
does not establish Unicode/IME input. Background key-release checks now passed
for all three languages in both profiles on arm64 and pinned x86_64. The current
Android backend does not populate the text-input character queue, and Malvado
does not provide an on-screen keyboard service. See
[keyboard verification](android.md#android-keyboard-verification).

## Delivered artifacts and unavailable paths

| Target | Current artifacts | Authoring and verification limits |
| --- | --- | --- |
| Native macOS / Linux | Installed C/Lua/Rust SDK and SDK archives; exported game folders; macOS also supports `.app` bundles | Native host builds. Rust development requires Cargo/rustc; exported players do not. Cross compilation is not established by native CI. |
| Windows x64 MSVC | Installed C/Lua/Rust SDK and SDK archives; exported game folders with `.exe` players | Native VS 2022 x64 authoring. No Windows `--app`, GNU/ARM64 support or Unix managed SDK installer. Players require a WGL provider and release CRT. |
| Android arm64 / x86_64 API 34 | Relocated C/Rust SDK trees, headless C/Lua/Rust executables and C/Lua/Rust NativeActivity fixture APKs | macOS arm64 or Ubuntu 24.04 x86_64 verification hosts only. C/Rust fixtures consume installed SDKs; Lua fixtures build the source runtime. Both profiles passed graphics/assets/error/input/audio/lifecycle fixtures on arm64 and pinned x86_64. Remaining mobile behavior and general game APK/AAB exports stay open. Fixture APKs use test signing, including Release builds. |

An installed Android C SDK consumer is an external CMake project. A Lua fixture
APK currently builds the source runtime; it does not establish an installed Lua
APK authoring flow. Headless executables run through adb and are not player
packages. Fixture APKs do not establish production signing, store publication,
other API levels, 32-bit ABIs or Android as an authoring host.

Use the same scripts locally and in CI: `mise run test-platform` for native
desktop SDKs, `mise run test-android` for the Android headless foundation and
`mise run test-android-window` for APK verification. The commands and required
flags are in the sections below and [Android verification](android.md).
Future artifact formats in the CLI specification remain planned until their
version checklist and executable acceptance checks are complete.

The macOS Apple Silicon Android inputs now have fixed archive commands for the
SDK, emulator 37.2.13 and API 34 arm64 image revision 14. Both profiles passed
headless C/Lua SDK/parity verification and normal/cancelled C/Lua/Rust fixture
APKs with recorded archive provenance. See the
[pinned macOS commands](android.md#local-commands). The earlier lifecycle and
audio results retain their own baseline; the new pin check does not complete
mobile behavior or general APK exports. Windows toolchain selection and the delivered-capability matrix review passed
A8.1 on `25bd953`. Production Android packaging remains A8.5 work.

## Reproducible command scope

The delivered matrix covers native macOS arm64, Ubuntu x86_64 and Windows x64
C/Lua/Rust authoring, plus macOS-arm64/Android-arm64 and Linux-x86_64/Android-x86_64
verification. Android runtime is API 34 with minimum native build API 28.
CMake, Python, Rust and signing Java are fixed in `mise.toml`; Windows compiler
selection is below. Android SDK/emulator/image names and SHA-256 values are in
`tools/android_sdk.py` and `tools/android_archives.py`; use the shared
[local and CI commands](android.md#local-commands). The AVD fixes two cores,
2048 MiB and a 1080 by 1920 display with SwiftShader.

SDK archives use `.tar.gz`; Android verification also delivers relocated SDK
trees and ELF executables. Desktop exports are folders, with `.exe` players on
Windows and optional `.app` bundles on macOS. Android fixture APKs are test
artifacts. General APK/AAB generation and its production packaging dependencies
remain A8.5 work. No Android CLI host, Windows-to-Android authoring, other Android
API levels, 32-bit target, Windows GNU/ARM64 or general cross-host desktop build
is established by this matrix. Hardware and audible checks remain beta work.
Hosted OS images are recorded baselines; version checks and archive hashes do
not promise a hermetic OS or byte-identical signed packages.

## Native Windows baseline

Use the Visual Studio 2022 C++ desktop workload on x64 with these exact inputs:

| Component | Required version |
| --- | --- |
| Visual Studio installation build | `17.14.37906.3` |
| MSVC toolset directory | `14.44.35207` |
| C compiler | `19.44.35230.0` |
| Windows SDK | `10.0.26100.0` |
| CMake / Python / Rust | Root `mise.toml` |

`mise run test-platform` selects the matching complete VS instance with
vswhere. It selects the x64 host/compiler, toolset and SDK through CMake's
standard generator environment. Child CMake, installed SDK, CLI and Cargo
commands inherit that selection. The preflight checks the actual CMake compiler,
SDK and VS build, then builds and runs an x64 C probe in Debug and Release.
It writes `windows-toolchain.log` and `windows-toolchain.json` in the work
folder; CI uploads both. A failed preflight removes stale success metadata and
its own short temporary probe directory.

Use a new `--work` directory when replacing a previous unpinned test tree.
Cached generator selections cannot bypass the requested pins. Missing versions
produce an installation error instead of a silent fallback to another compiler.
Install the listed VS build, C++ workload/toolset and Windows SDK before retrying.
This does not install tools or claim a hermetic Windows image. The hosted runner
can change; a removed pinned version requires a deliberate baseline review.
Native selection acceptance passed on clean `25bd953` in
[Windows run 44054785505](https://github.com/jepemo/malvado/actions/runs/44054785505).
Both uploaded toolchain JSON files match every pin. Both SDK kinds/profiles also
passed Rust selector-change rejection/recovery, and fresh exported players ran.
The same source passed [general CI](https://github.com/jepemo/malvado/actions/runs/44054785682)
and [Android CI](https://github.com/jepemo/malvado/actions/runs/44054785787).

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
The initial [Android foundation](android.md) records its NDK, emulator and commands.
Complete archive pins and the host matrix remain part of A8.1; do not infer
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
consumers and SDK diagnostics. Windows CLI and exports passed the A8.3 verification described below.

Native Rust SDK verification also checks Cargo cache refresh. After a successful
installed consumer check, change the CMake platform, toolset and instance one
at a time to invalid selections. Each must fail. Restoring the selected
toolchain must pass again. The shared platform command runs these checks in
both profiles; `mise run test-rust` also covers the installed core consumer.
The tests select an explicit generator so CMake applies the other environment
defaults. ABI validation must run again instead of accepting a stale Cargo
result. Unrelated compiler failures do not satisfy the expected rejection.

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

Native verification passed on `a60be46` in
[run 43449424295](https://github.com/jepemo/malvado/actions/runs/43449424295),
closing A8.3. The graphical matrix
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

Both SDK profiles passed authoring checks. The CLI builds its existing Debug
game target against each SDK profile; profile labels do not imply a new CLI
release-build option. All six C/Lua/Rust exports then passed on a separate runner,
including nonzero status when an asset was removed. Paths use ASCII names and
spaces in this baseline; Unicode CLI project/SDK paths remain unverified.
C `check` configures/builds/links without running the game, including rejection
of invalid C. Rust SDK probes use short owned temporary directories on Windows
to avoid MSBuild's nested compiler-identification path limit, with the required
helper archive retained under Cargo output before cleanup.

Graphical Linux SDK consumers require X11 in addition to OpenGL and threads.
Raylib 6.2 uses Xlib directly for clipboard image handling. The installed CMake
package discovers `X11::X11`; Rust links `X11`. Core-only and Android SDKs do not
require X11. The `libx11-dev` authoring prerequisite already exists in CI.

The full Android Rust fixture matrix also passed in pinned x86_64 CI on
`9c96692`, including assets, expected failures, Back/resume, contact, keyboard
and explicit audio pause continuity. Downloaded evidence contains 38 distinct
APK packages and 58 PNG headers per profile across C/Lua/Rust. Windows passed
on that revision. General Linux installed SDK consumers exposed the missing
X11 link dependency fixed separately; the new exact candidate still needs CI.

The X11 and Java follow-up source `46a45cd` passed all 15 general CI jobs,
Windows and both Android profiles. Downloaded Android results identify the
pinned Temurin 21.0.7+6 JDK for all 38 APK cases per profile. The current
background-key extension passed eight local arm64 cases and native regressions;
its x86_64 CI acceptance passed on `fc59b77`. Export/startup/focus gaps remain
in the version checklist.

Background contact cancellation now passes eight local arm64 cases across C,
both Lua entries and Rust in Debug/Release. It requires language-specific held,
movement and release markers, the same PID, restored pixels and cleanup. The
new pointer source still needs x86_64 CI. It does not add multi-touch or physical
mouse/device acceptance.

## Windows SDK candidate archive

The graphical Windows verifier uses `--package` to create one Release SDK
archive and SHA-256 file in `build-platform-graphics/sdk-package`. It extracts
the archive to the SDK path with spaces before installed Rust consumers and
C/Lua/Rust CLI checks. The archive uses the existing SDK format and notices;
it contains no test Mesa driver. The Windows workflow uploads it as
`sdk-windows-COMMIT_SHA` after all authoring checks pass. Fresh exported players
must still pass in the separate Windows job before candidate acceptance.

Use `mise run test-platform -- --package --rust` for a headless native check.
Add the existing Windows `--graphics --cli` options for the graphical SDK.
`--package` verifies the digest, metadata and safe extraction paths before
using the relocated archive. It preserves an existing extraction destination.
Windows archive execution remains subject to hosted native CI acceptance.
