# Platform support and verification

The [alpha.8 candidate](../versions/2.0.0-alpha.8.md#accepted-candidate)
established these paths. The [beta.1 release](../versions/2.0.0-beta.1.md)
passed the same platform gate on its release revision.

| Host / target | Current verification | Follow-up |
| --- | --- | --- |
| macOS 15 arm64 / native | C/Lua/Rust and graphics in existing CI | Keep checks passing |
| Ubuntu 24.04 x86_64 / native | C/Lua/Rust, graphics and clean exports in existing CI | Keep checks passing |
| Windows Server 2022 / x64 MSVC | Verified headless and Mesa graphics C/Lua/Rust SDKs in Debug and Release | Verified CLI/export and fresh players; manual hardware review remains open |
| macOS arm64 / Android arm64 API 34 | Headless core/assets, graphical fixtures, and C/Lua/Rust production APKs and AAB-derived APKs in Debug/Release as applicable. Exported physical-key text passed in both APK profiles. | Physical-device keyboard and audible checks |
| Ubuntu 24.04 x86_64 / Android x86_64 API 34 | Headless core/assets and C/Lua/Rust graphical fixtures in Debug/Release. Includes input, explicit audio pause, recreation, overlay relaunch and cold restart. | Emulator fixture coverage |
| Ubuntu 24.04 x86_64 / Android arm64 API 34 | Hosted C/Lua/Rust APKs and AAB-derived APKs ran through native translation in both profiles. Rust ran executable ABI probes. The IME and audio-focus tests passed in both profiles. | Physical-device keyboard and audible checks |

## Manual verification limits

Windows x64 and Android arm64 have beta platform support in 2.0.
Windows has automated SDK and export coverage. The manual SDK upgrade and
removal flow, real graphics drivers, physical controllers and audible
playback remain unverified.

Android emulators cover package launch, input and lifecycle. Physical
devices still need checks for touch and rotation, Home and Back, hardware
keyboard and system IME input, and audible playback across backgrounding.
Report failures with the SDK revision, OS, hardware and steps to reproduce.

## Delivered artifacts and unavailable paths

| Target | Current artifacts | Authoring and verification limits |
| --- | --- | --- |
| Native macOS / Linux | Installed C/Lua/Rust SDK and SDK archives; exported game folders; macOS also supports `.app` bundles | Native host builds. Rust development requires Cargo/rustc; exported players do not. Cross compilation is not established by native CI. |
| Windows x64 MSVC | Installed C/Lua/Rust SDK and SDK archives; exported game folders with `.exe` players | Native VS 2022 x64 authoring. No Windows `--app`, GNU/ARM64 support or Unix managed SDK installer. Players require a WGL provider and release CRT. |
| Android arm64 / x86_64 API 34 | Relocated C/Rust SDK trees, headless C/Lua/Rust executables and C/Lua/Rust NativeActivity fixture APKs | macOS arm64 or Ubuntu 24.04 x86_64 verification hosts only. C/Rust fixtures consume installed SDKs; Lua fixtures build the source runtime. Both profiles passed graphics/assets/error/input/audio/lifecycle fixtures on arm64 and pinned x86_64. Fixture APKs use test signing, including Release builds. |
| Android arm64 API 34 | Installed-CLI C/Lua/Rust debug APKs and release APKs/AABs | Verified on macOS arm64 with a native image and on Ubuntu x86_64 with translated execution. Windows Android authoring is unavailable. |

## Run platform checks

- Desktop SDKs: `mise run test-platform`.
- Clean macOS and Linux desktop players: [desktop walkthrough](desktop-validation.md).
- Android: [emulator and fixture commands](android.md#local-commands) and
  [production package checks](ci.md#android-production-packages).
- Release candidates: [exact-revision gate](ci.md#supported-platform-candidate-gate).

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
