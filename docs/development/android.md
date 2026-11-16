# Android foundation verification

Android support is in progress for alpha.8. The current delivered path is an
NDK-built headless C/Lua runtime and C/Lua NativeActivity graphics on an Android
emulator. These are verification paths, not an Android CLI authoring host or
a CLI APK exporter. Rust graphical startup, general packaged-game integration, experimental
Rust runtime, CLI APK/AAB exports and mobile
behavior remain open in the [version plan](../versions/2.0.0-alpha.8.md).

## Tested baseline

| Component | Verified configuration |
| --- | --- |
| Authoring host | macOS, Apple Silicon |
| NDK | r27c, `27.2.12479018` |
| Native ABI / minimum build API | `arm64-v8a` / API 28 |
| Runtime | Android 14, API 34 |
| System image | `system-images;android-34;google_apis;arm64-v8a`, revision 8 |
| Emulator | 33.1.23, build 11150993 |
| AVD | Pixel 2, two cores, 2048 MiB, SwiftShader, no window/audio/snapshots |
| Android platform tools | 34.0.5 |
| Android command-line tools | 12.0, build 11076708 |
| CMake / Python | Versions pinned in root `mise.toml` |
| Java used for AVD creation | Temurin 21.0.7 |
| Profiles | Debug and Release, assertions enabled in verification fixtures |

API 28 is a compilation baseline. These results establish runtime behavior on
API 34 only. They do not establish other Android releases, 32-bit ABIs or devices.
Linux x86_64 to Android API 34 `x86_64` also passed both profiles in
[run 43833030421](https://github.com/jepemo/malvado/actions/runs/43833030421)
on `873122b`. That run used Ubuntu 24.04, emulator 37.2.13, platform tools
37.0.1 and Google APIs image revision 14. Its native API was also 28. The recorded
fingerprint is `google/sdk_gphone64_x86_64/emu64xa:14/UE1A.230829.050/12077443:userdebug/dev-keys`. Android authoring from
Windows is not verified. Native desktop support remains documented in the
[platform matrix](alpha8-platforms.md).

The NDK is an exact prerequisite and is checked before configuration. The
verifier records the actual emulator and adb revisions in `toolchain.json`.
SDK Manager can update system images and emulator packages. A different package
revision is a new verification configuration, not evidence for the baseline
above. Finishing archive pins and the broader host matrix remains A8.1 work.
No Gradle, signing or store toolchain is claimed by this headless gate.

## Local commands

Install [Android command-line tools](https://developer.android.com/tools) and
[NDK r27c](https://github.com/android/ndk/releases/tag/r27c). Set
`ANDROID_SDK_ROOT` to the installed SDK. Use
[`sdkmanager`](https://developer.android.com/tools/sdkmanager) to install the NDK
and API 34 image. The command below selects packages; record their revisions.
It does not force an old image revision that SDK Manager no longer serves.

```sh
"$ANDROID_SDK_ROOT/cmdline-tools/12.0/bin/sdkmanager" \
  "ndk;27.2.12479018" "system-images;android-34;google_apis;arm64-v8a"
mkdir -p build-android/avd
ANDROID_AVD_HOME="$PWD/build-android/avd" \
  "$ANDROID_SDK_ROOT/cmdline-tools/12.0/bin/avdmanager" create avd \
  --name malvado-alpha8 --package "system-images;android-34;google_apis;arm64-v8a" \
  --device pixel_2
ANDROID_AVD_HOME="$PWD/build-android/avd" \
  "$ANDROID_SDK_ROOT/emulator/emulator" -avd malvado-alpha8 -port 5580 \
  -no-window -no-audio -no-boot-anim -no-snapshot \
  -gpu swiftshader_indirect -cores 2 -memory 2048
```

Run the emulator in a separate terminal. After `sys.boot_completed` is `1`, run:

```sh
mise run test-android -- --serial emulator-5580 --preflight
mise run test-android -- --serial emulator-5580
```

`--sdk PATH` overrides `ANDROID_SDK_ROOT`. `--config Debug` or `--config Release`
selects one profile. The default runs both. The selected serial is mandatory;
the verifier rejects physical devices, an unfinished boot and a different API.
It does not start, reset or stop a user's AVD. Stop the dedicated emulator after
verification with `adb -s emulator-5580 emu kill`.

## What the checks establish

The script builds the existing C core, asset services, bundled Lua and the
headless adapter with the NDK. It discovers native executable checks from CTest,
copies committed fixtures to a unique device directory, and runs the binaries
there. Host CMake/Python/shell checks and desktop CLI commands are excluded.
Each command has a timeout. Expected output, failure status and test environment
are preserved. A signal or remote shell error cannot satisfy an expected failure.
The script removes only its own device directory, including after a failure.

The first baseline passed 156 C/Lua executable checks per profile. These cover
scheduler timing, process relationships, cancellation, handles, resources,
allocation failures, collision, assets, text, both Lua entry forms and mixed
C/Lua ownership. They use the same C scheduler as desktop builds. Source assets
are ordinary files copied into an adb test directory; these checks do not verify
APK assets or application sandbox startup.

Android has no writable `/tmp`. The bundled Lua build uses its existing atomic
`mkstemp` implementation with `lua_XXXXXX` in the working directory. The host
must enter a writable game directory before running Lua. `os.tmpname()` returns
a relative path in this configuration. The asset-loader fixture verifies temp
creation, file writes, reads and removal on Android. No third-party source is
modified.

The verifier also installs and relocates each Android SDK to a path with spaces.
It builds an external C project from a separate source directory with installed
headers and `Malvado::core` / `Malvado::assets`. The emulator receives only its
executable. It checks process execution, handles after context shutdown, binary
asset snapshots after file removal and cleanup of an unfinished process. SDK
metadata and Malvado/Lua/cJSON notices must be present. Both profiles passed.
An explicitly selected target SDK outside the NDK sysroot needs
`NO_CMAKE_FIND_ROOT_PATH` on its `find_package` call; see
`tests/android-sdk/CMakeLists.txt` for the working consumer.

Nine existing CMake suites also run the Android Lua executable through a bounded
adb adapter. These check invalid game tables, startup results, cleanup after
load/init/process errors, tracebacks, shutdown handles and four close-failure
paths. Generated scripts are copied to the verifier's own directory. stdout,
stderr and exit status reach the original contract checks. The same adapter
compares six C/Lua traces with the shared fixtures: bound colliders, camera
following, worlds, fades, lifecycle and the alpha.7 baseline. Both profiles
passed all 15 suites. This does not verify Android Activity lifecycle events.

Logs and JSON results remain under `build-android`, including on failure.
They record the source commit and the actual tools. Uncommitted changes are not
a release-candidate proof. Headless tests do not establish graphics, touch,
Back, pause/resume, audible output or physical devices. Those gates stay open.

## Continuous verification

`.github/workflows/android.yml` runs this same verifier on Ubuntu 24.04 with
KVM and an isolated API 34 Google APIs `x86_64` emulator. It runs both profiles
on pushes, pull requests, manual requests and weekly. Bounded C/Lua test APKs
run after the headless suite. No store key or production signing credential is used. AVD boot has a 240-second limit;
command and job timeouts remain active. The job uploads emulator/build/test
logs and tool/result JSON even after failure.

NDK r27c is pinned. Linux CI uses checksum-pinned emulator and image archives
as described below. SDK Manager supplies the remaining authoring tools.
Complete toolchain archive pins remain A8.1 work. The exact-commit run linked above passed 156 native checks, one installed-SDK
consumer and 15 contract/parity suites per profile, with no uncommitted files.
These checks do not close A8.7's Rust, mobile, general export or candidate acceptance gates.

## Pinned Linux emulator archives

`tools/android_archives.py --output build-android/pinned` downloads emulator
37.2.13 (build 16740729) and Google APIs API 34 x86_64 image revision 14 from
Google. Each archive has a SHA-256 digest in the script. Digests were computed
after checking the archive against Google's repository checksum. The preparer
checks the complete digest before extraction, rejects unsafe paths and symlinks,
retains executable permissions, and refuses an existing destination. Downloads
and staging are owned temporary files and are removed on success or failure.
The full archives were downloaded, verified and extracted locally.

The Android workflow uses the extracted emulator executable and sets the AVD's
`image.sysdir.1` to the extracted image. SDK Manager still provides the pinned
NDK version and the command-line/platform tools. It no longer selects the
emulator or image used to run the tests. Remaining authoring-tool archive pins
and the full host/target matrix stay in A8.1. No Docker images are created.

Pinned-input execution passed Debug and Release on `80b95cd` in
[run 43840474589](https://github.com/jepemo/malvado/actions/runs/43840474589).
Each profile passed 156 native checks, one installed SDK consumer and 15
contract/parity suites. The local 5 GiB extracted verification copy was removed
after testing. Archive failure regressions confirm that owned temporary downloads
are cleaned while unrelated files remain intact.

## C NativeActivity window verification

`mise run test-android-window -- --sdk "$ANDROID_SDK_ROOT" --serial emulator-5580`
builds a C NativeActivity APK with the existing Android raylib backend. It checks
startup, a red framebuffer pixel, bounded rendering and window shutdown, then
uninstalls its unique test package and verifies removal. Commands have timeouts;
logs and results remain in `build-android-window-test`. Build-tools 34.0.0,
platform android-34 and Java/keytool are required. The local signing key is for
tests only. This is not a CLI exporter or a production signing configuration.

The arm64 API 34 emulator passed. Android SDK archives use PIC and select raylib's
Android backend automatically. The pixel fixture reads the actual render surface:
raylib 5.5's screen-image helper uses a DPI query that Android does not implement.
No third-party source was changed. This gate does not establish Lua/Rust window
startup, APK assets, input, Activity lifecycle or physical-device behavior.

Add `--installed-sdk` to install and relocate the graphical SDK, copy the C
consumer to a separate project, link through `Malvado::graphics`, and run its
APK. This passed on Android arm64. Imported Android graphics dependencies are
EGL, GLESv2 and Android system libraries. The NativeActivity entry point remains
linked. Desktop OpenGL discovery is unchanged for desktop consumers.
`--config Release` selects an optimized SDK and consumer; Debug is the default.

The installed graphical consumer passed both profiles on x86_64 in
[run 43845179661](https://github.com/jepemo/malvado/actions/runs/43845179661),
commit `1b9012e`. The same fixtures passed Debug and Release on local arm64.
The workflow uploads each window log and result JSON. A unique token prevents
old logcat messages from satisfying a new run. Failed runs retain crash-buffer
diagnostics. Results are written only after package removal is verified.

## Lua NativeActivity and packaged assets

Use `mise run test-android-window -- --sdk "$ANDROID_SDK_ROOT"
--serial emulator-5580 --language lua --entry explicit`. Select
`--entry declarative` for a returned game table, and `--config Release` for
the optimized native build. The two forms use the same runtime, native
scheduler and behavior module. Both profiles passed on the arm64 API 34 emulator.

The test APK stores its scripts, PNG, atlas metadata and binary payload directly
in APK assets. Its fixed-allowlist C host reads these with Android AssetManager
and copies them into its private application directory before starting Lua.
The existing file APIs then read them without a new process API or asset VFS.
The checks cover binary NUL/CRLF/255 bytes, atlas lookup, atomic temporary files,
a framebuffer pixel, exactly-once process closure, and asset snapshot bytes
after source removal. The APK is uninstalled after each run.

This test bootstrap is not a general extraction API or CLI export implementation.
Lua APK verification currently builds from source; `--installed-sdk` remains a
C-only check. APK fixtures are debuggable, with a local test signing key. Native
Release profile tests do not establish production release APK signing. Failure
evidence includes the application's captured Lua stderr before uninstall.

Use `--failure process` or `--failure missing-image` to check expected errors.
The first mode fails after visible frames; the second omits only the PNG while
keeping its atlas metadata. Both entry forms passed both negative cases in
Debug and Release on arm64. A run passes only with the expected runner status
and diagnostic, exactly one close marker, no live window/context, a valid C asset
snapshot after file removal and successful uninstall. The process-error case
also requires the framebuffer pixel check before the error.

`tests/android_lua_package.py` checks the exact entry script and binary bytes,
direct APK asset paths and image presence for each mode without an emulator.
These host checks do not replace native execution. The helper rejects invalid
Lua modes before mutating the archive.

The Lua framebuffer check observes presented frames until the process terminates.
It requires the red pixel but does not require 30 rendered frames within 60
simulation ticks. A slow emulator can run several native simulation steps per
presented frame. This preserves the existing clock and scheduler contracts.

The pinned x86_64 emulator passed all twelve Lua APK combinations on `e33d844`
in [run 43849456706](https://github.com/jepemo/malvado/actions/runs/43849456706).
The workflow retains a separate log and JSON for each entry/profile/failure case,
including the current log when a case fails before a result can be written.
Local host verification passed 206 headless checks, the APK payload regressions
and 673 guide links. The installed C graphical consumer also passed again.
