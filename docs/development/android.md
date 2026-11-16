# Android foundation verification

Android support is in progress for alpha.8. The current delivered path is an
NDK-built, headless C/Lua runtime on an Android emulator. This is an internal
verification path, not an Android CLI authoring host or an APK exporter.
Window startup, packaged assets, experimental Rust, APK/AAB exports and mobile
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
The verifier also accepts an API 34 `x86_64` emulator for Linux CI. That path must
pass its own checks before it becomes a support claim. Android authoring from
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
on pushes, pull requests, manual requests and weekly. No Android app, store key
or production signing credential is used. AVD boot has a 240-second limit;
command and job timeouts remain active. The job uploads emulator/build/test
logs and tool/result JSON even after failure.

NDK r27c is pinned. The hosted SDK Manager currently supplies the emulator,
platform tools and image revisions; each run records them. Exact download pins
remain A8.1 work. A configured job alone does not establish Linux-to-Android
support. Its exact-commit result must pass before that claim is added. This
headless job does not close A8.7's graphical/mobile or candidate acceptance gates.
