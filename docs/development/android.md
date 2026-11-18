# Android foundation verification

Android support is in progress for alpha.8. The delivered verification paths
include headless C/Lua/Rust core and assets, C/Lua NativeActivity graphics, and
local Rust NativeActivity startup/pixels/cleanup. These are verification paths,
not an Android CLI authoring host or a general APK exporter. Pinned x86_64 Rust
fixtures also pass CI. General packaged-game integration,
CLI APK/AAB exports and remaining mobile behavior stay open in the
[version plan](../versions/2.0.0-alpha.8.md).

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
| Java for APK signing | Temurin 21.0.7+6, pinned in `mise.toml` |
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
APK results also record Build Tools, the API platform revision/extension and
Java. An isolated SDK from `tools/android_sdk.py` adds its archive provenance
to the results. APK verification rejects a mislabeled Build Tools directory
or a platform other than API 34 before configuration.
The verifier installs complete APKs with `adb install --no-incremental`.
Incremental APK loading is not part of this runtime baseline.
SDK Manager can update system images and emulator packages. A different package
revision is a new verification configuration, not evidence for the baseline
above. Finishing archive pins and the broader host matrix remains A8.1 work.
No Gradle, signing or store toolchain is claimed by this headless gate.

Linux CI uses `build-android/linux-sdk` for the NDK, adb, Build Tools and
`android.jar`. It links the separately pinned emulator/image into that SDK.
It stops the runner's previous adb server and starts the fixed adb before
verification. The shared `tools/android_avd.py` command creates fixed AVD metadata directly
from the selected image. It does not require SDK Manager, avdmanager or Java.
Java for APK signing is pinned to Temurin 21.0.7+6 in `mise.toml`.
Run `mise install java`; use `mise run test-android-window` to select it. The
verifier rejects a different patch, build or provider before configuration.
Direct AVD creation does not need Java or macOS system-wide JDK registration.

## Local commands

For the Linux x86_64 verification SDK, use the fixed archive installer:

```sh
python3 tools/android_sdk.py --output build-android/linux-sdk
python3 tools/android_archives.py --output build-android/pinned
```

The first command installs NDK r27c, platform tools 37.0.1, Build Tools 34.0.0
and API 34 extension 7 revision 3. The second installs the pinned emulator and
image. Both commands require a new destination. They verify SHA-256 before
extraction, remove their temporary downloads on failure and preserve existing
SDKs. `linux-sdk/malvado-archives.json` records archive names and digests.
The NDK archive contains internal file links; only links to archived files are
accepted. This SDK executes on Linux x86_64. Extracting it on another host does
not establish cross-host tool support. Google archive licenses remain in each
package. Java remains a host prerequisite for APK signing.

For macOS Apple Silicon, install the matching SDK archives:

```sh
python3 tools/android_sdk.py --host macos --output build-android/macos-sdk
```

This selects the Darwin NDK, adb and Build Tools archives. The platform archive
is shared with Linux. The default host stays `linux` for existing CI commands;
`--host` selects archive contents and does not change the running host. The
macOS SDK retains the same exclusive destination, checksum, extraction and
cleanup rules. It records all four archive names and SHA-256 values in
`malvado-archives.json`. Use the mise-selected Java for APK signing.

For the pinned macOS Apple Silicon emulator and API 34 arm64 image, use:

```sh
python3 tools/android_archives.py --host macos --output build-android/macos-runtime
export ANDROID_HOME="$PWD/build-android/macos-sdk"
ln -s "$PWD/build-android/macos-runtime/emulator" "$ANDROID_HOME/emulator"
mkdir -p "$ANDROID_HOME/system-images/android-34/google_apis"
ln -s "$PWD/build-android/macos-runtime/arm64-v8a" \
  "$ANDROID_HOME/system-images/android-34/google_apis/arm64-v8a"
python3 tools/android_avd.py --output build-android/macos-avd \
  --image build-android/macos-runtime/arm64-v8a
ANDROID_SDK_ROOT="$ANDROID_HOME" ANDROID_AVD_HOME="$PWD/build-android/macos-avd" \
  "$ANDROID_HOME/emulator/emulator" -avd malvado-alpha8 -port 5584 \
  -no-window -no-audio -no-boot-anim -no-snapshot \
  -gpu swiftshader_indirect -cores 2 -memory 2048
```

Keep the emulator running and use another terminal for the shared verification
commands. Set `ANDROID_HOME` there to the same absolute SDK path:

```sh
python3 tools/android_boot.py --adb "$ANDROID_HOME/platform-tools/adb" --serial emulator-5584
mise run test-android -- --sdk "$ANDROID_HOME" --serial emulator-5584
mise run test-android-window -- --sdk "$ANDROID_HOME" --serial emulator-5584 --installed-sdk
"$ANDROID_HOME/platform-tools/adb" -s emulator-5584 emu kill
```

The pinned runtime is emulator 37.2.13 build 16740729 and Google APIs arm64 API
34 image revision 14. This is a new reproducible baseline; the revision 8 image
in the historical table above is no longer served by SDK Manager. The archive
installer records host, ABI, names and SHA-256 values in
`macos-runtime/malvado-emulator-archives.json`. Verification follows the emulator
link to include that provenance in its toolchain and APK results. Existing Linux
commands keep their x86_64 archives and now record the same runtime provenance.

The pinned macOS baseline passed Debug and Release target verification: 156
native C/Lua checks, installed C SDK consumers and 15 startup/parity suites per
profile. Sixteen distinct APKs passed normal and cancelled startup for C, both
Lua entries and Rust, with assets, scheduler/shutdown checks and verified APK
removal. Every result retains both SDK and runtime archive provenance. These
checks establish this focused baseline; the historical lifecycle/audio evidence
above does not establish all mobile behavior on the new image automatically.

Create a dedicated AVD directory with the same command used by CI:

```sh
python3 tools/android_avd.py --output build-android/isolated-avd --image build-android/pinned/x86_64
```

Set `ANDROID_AVD_HOME` to that absolute output path when starting the emulator.
The command accepts API 34 Google APIs arm64-v8a and x86_64 images, requires
system/userdata/ramdisk files and refuses an existing output directory. It fixes
1080×1920 pixels, density 420, two cores and 2048 MiB. It does not download or
verify image archives; use the checksum-verified installer above for either host.
The historical commands below remain an alternative for host-managed images.

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

Run the emulator in a separate terminal. Wait for it with the CI boot check:

```sh
python3 tools/android_boot.py --adb "$ANDROID_SDK_ROOT/platform-tools/adb" --serial emulator-5580 --timeout 240
```

This command selects one emulator, retries transient adb timeouts and requires
`sys.boot_completed=1` with a successful exit status. The deadline bounds each
adb command and retry delay. A failure reports the last response. A successful
boot check does not establish APK startup or graphical behavior. Then run:

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
Debug and Release run in independent jobs with fail-fast disabled. Each owns
its emulator and uploads `android-foundation-COMMIT-PROFILE`. Candidate review
requires both profiles; a failed Debug job cannot cancel Release evidence.

Linux CI uses checksum-pinned NDK, adb, APK tools, platform, emulator and image
archives. The shared direct-AVD command needs no command-line AVD manager;
APK signing uses the pinned Java toolchain.
Their reproducibility review and the Windows toolchain remain A8.1 work.
The exact-commit run linked above passed 156 native checks, one installed-SDK
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
`image.sysdir.1` to the extracted image. The SDK archive installer supplies the
fixed NDK, adb, Build Tools and API platform. SDK Manager supplies metadata for
host AVD management; it does not select compilation or runtime archive inputs.
The full reproducibility review and host/target matrix stay in A8.1.
No Docker images are created.

The complete isolated SDK passed on `75c973c` in
[run 43884605632](https://github.com/jepemo/malvado/actions/runs/43884605632).
Both profiles passed 156 native checks and 15 contract/parity suites. The 44
graphical APK cases used the same four SDK archive digests, Build Tools 34.0.0,
API platform revision 3/extension 7 and complete APK installation. Results
record Temurin 17.0.21+9 for the host Java prerequisite. The downloaded artifact
also passed the per-case metadata, same-PID resume and PNG-header review.

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

## Back and Activity lifecycle checks

`--lifecycle back` keeps the C or Lua fixture alive until the host injects Back.
Android Back now requests closure through the existing platform window query.
C remains responsible for its host loop; Lua uses its existing cancellation and
resource cleanup path. The backend forwards events to raylib and restores its
input handler on window close. This is a platform adaptation, not a new public
input operation.

C installed-SDK and both Lua entry forms passed this mode in Debug and Release
on the API 34 arm64 emulator. A normal timeout cannot satisfy the test: the C
fixture requires a close request and the Lua fixture runs until cancellation.
Both retain their pixel/shutdown checks and verify package removal.

`--lifecycle resume` additionally drives Home and foreground restoration. The
fixture chains the existing native Activity callback for observation. It must
see pause and resume, retain the same PID and read the expected framebuffer
pixel after resuming before Back can finish the test. This does not establish
OS process-death recovery, audio behavior, touch, physical devices or all OEM
Activity implementations. Lifecycle modes cannot be combined with expected
Lua failures.

The resume mode passed all six C/Lua entry/profile cases on arm64. The host
requires known resumed-Activity fields before accepting a background transition,
and rejects a changed PID before sending Back. Result JSON records the initial
and resumed PID. The native fixture requires actual pause/resume callbacks and
a red pixel after resume, rather than only a successful `am start` command.
Host regression tests cover unknown Activity output and process recreation.

All twelve lifecycle cases passed on the pinned x86_64 emulator in
[run 43852748730](https://github.com/jepemo/malvado/actions/runs/43852748730),
commit `51d609f`. Separate logs and JSON cover C installed-SDK and both Lua
entries, Debug/Release, Back and resume. Each resume result records identical
initial/resumed PIDs. The existing C/Lua APK and headless checks also passed.
The injected Back key is verified; gesture navigation remains device review.
This adds no process-death persistence contract or automatic audio-pause claim.

Desktop regression verification passed 207 standard and 207 headless checks.
Fifteen Android verifier/payload/archive regressions and 674 guide links passed.
The dedicated local emulator was stopped and no test APK remained installed.
No Docker images were created.

## Primary contact input

Use `--lifecycle touch` with the C installed-SDK consumer or `--language lua`
and either entry form. The host holds a contact at the physical screen center,
moves it right and releases it. Screenshot dimensions supply the rotated display
coordinates. The existing C mouse API and the Lua mouse API must observe left
down, logical center coordinates, movement and release. Middle and right remain
up. Lua must write its own release marker before Back can close the app.

All six entry/profile cases passed locally on arm64. The contact lasts 1500 ms;
this verifies sampled held state, not short-tap buffering, multi-touch or gesture
recognition. The current backend already supplies this mapping. No new public
mobile input API, scheduler or native property copy is introduced.

## Native screenshot evidence

Lifecycle runs retain `before.png`; resume and touch runs also retain
`resumed.png` or `released.png`. `adb exec-out screencap -p` supplies binary PNG
bytes without terminal newline conversion. Dimensions come from the screenshot
header, not an assumed portrait display size. The scene can change display
orientation during Activity transitions; framebuffer checks remain in logical
window coordinates. Six local resume cases passed with native screenshots.

The capture helper validates the PNG signature, IHDR size and positive bounded
dimensions before publication. A failed capture removes only its partial file
and preserves an existing destination. Each verification run clears its known
base screenshot names and old result JSON before starting. Host regressions
cover malformed geometry, binary preservation and timeout cleanup. Screenshots
are evidence alongside the native pixel checks, not a replacement for them.

The pinned x86_64 emulator passed all six contact entry/profile cases on
`55ba294` in [run 43872267669](https://github.com/jepemo/malvado/actions/runs/43872267669).
The artifact retains distinct images, logs and results per interaction. All
38 native PNG artifacts were checked for valid headers, including the before
and release image for each touch case. Existing Back/resume, C graphical SDK,
Lua assets/errors and headless suites passed in the same run.

Host verification passed 208 headless tests, 18 Android verifier regressions and
675 guide links. All local test APKs were uninstalled and the dedicated emulator
was stopped. No Docker images were created. Audio and the remaining input/device
coverage stay open; no multi-touch or gesture-recognition API is promised.

## Android keyboard verification

Use `--lifecycle keyboard` to verify physical-key state and transitions. Use
`--lifecycle keyboard-resume` to run the same checks after Home and foreground.
Both modes accept the C installed-SDK fixture or either Lua entry, in Debug and
Release. For example:

```sh
mise run test-android-window -- --sdk "$ANDROID_SDK_ROOT" --serial emulator-5580 \
  --installed-sdk --config Release --lifecycle keyboard-resume
mise run test-android-window -- --sdk "$ANDROID_SDK_ROOT" --serial emulator-5580 \
  --language lua --entry declarative --config Debug --lifecycle keyboard
```

The host injects Android A and D-pad Left long presses after the framebuffer is
ready. Native code and Lua each require held state, press/release snapshots,
consistent repeated reads and a later neutral tick. Lua writes ordered sandbox
markers; native success cannot substitute for Lua observations. Each case ends
through Back with normal resource cleanup. Resume must preserve the PID.
The native Lua observer runs between logical ticks. A bounded 100 ms fixture
stall requires multiple ticks between rendered frames and has its own acceptance
marker. A render callback can miss a tick transition and is used only for
graphics and the Lua completion files. Engine timing is unchanged.

Failed launches retain the full available filtered startup log, including the
NDK glue's `threaded_app` tag, plus entry/window/runtime stage markers. A bounded
`debuggerd -b` query attempts a native stack for the single selected APK PID.
Unavailable stack permissions or diagnostic timeouts preserve the original
failure. This diagnostic does not establish a fix for the recorded startup stall.

The fixture manifest handles `orientation`, `screenSize`, `screenLayout` and
`smallestScreenSize`, plus `keyboardHidden`. These are the size-related changes
documented by [Android](https://developer.android.com/guide/topics/resources/runtime-changes).
The packaged manifest is dumped with aapt before installation. A host regression
requires all four size flags. Startup evidence showed an Activity relaunch and
`APP_CMD_DESTROY` before `APP_CMD_INIT_WINDOW`; raylib 5.5 waits for a window in
that state. The pre-surface guard below now handles initial cancellation. This historical
manifest acceptance does not establish process-death recovery.
The observed local relaunch event used
[`CONFIG_ASSETS_PATHS`](https://developer.android.com/reference/android/R.attr#configChanges)
(`0x80000000`),
not a size flag. Cold-start verification now waits for
`am wait-for-broadcast-idle` after APK installation and requires its positive
completion message. This separates the baseline from in-flight package-added
resource updates. Waiting for broadcasts does not verify runtime relaunch
recovery. The separate live resource-overlay checks below cover that path.
Initial destruction is verified separately below.
Results record keycodes 29/21 and capture a PNG after release, plus before/resumed
evidence where applicable. CI retains distinct logs, JSON and PNGs for each case.

The arm64 API 34 emulator passed all twelve entry/profile/mode cases. These
checks do not establish a physical keyboard, key release while backgrounded,
Unicode typing or IME composition. At that baseline, raylib 5.5 Android did not
fill its character queue. The physical-key text extension below now fills
Malvado's shared queue. No on-screen keyboard service is delivered. Malvado derives observed
key edges from held state to avoid raylib's incomplete navigation-key history;
this keeps the existing [tick snapshot contract](../api/keyboard.md).

All twelve cases also passed on the pinned x86_64 emulator on `a7f850e` in
[run 43897792937](https://github.com/jepemo/malvado/actions/runs/43897792937).
Both independent profile jobs passed their complete 28-case APK matrices and
headless suites. Artifact review confirmed all 56 APK results, positive package
broadcast-idle evidence, compiled size-change flags, eight Lua catch-up markers
and 92 PNG headers. This baseline did not establish initial-window cancellation or runtime overlay
recovery. Initial cancellation is covered separately below.

## Android audio API verification

Add `--audio` to the C installed-SDK or Lua APK verifier. The test packages silent
WAV/OGG and empty-WAV fixtures directly as APK assets. Its fixed bootstrap places
them in the application sandbox. Both C and Lua entry forms passed Debug and
Release on the arm64 API 34 emulator.

Checks cover device opening, invalid paths/empty files/volume, independent sound
clones after source release, playback, explicit pause/resume/stop and finite
sound completion. C destroys its owned audio objects before device close. Lua
uses weak references to verify that paused playback roots survive collection,
then leaves roots for runner shutdown to release. The native host requires the
audio device to be closed after Lua shutdown.

The emulator uses silent media and runs with host audio disabled. These results
establish API state and ownership, not audible output, latency or physical-device
audio focus. Native Release profile APKs still use the test signing configuration.
No new audio API, automatic background-pause policy or production exporter is
introduced. `--audio` cannot be combined with expected Lua failure fixtures.

Combine `--audio --lifecycle resume` to keep explicitly paused sound/music alive
across Home and foreground restoration. All six C/Lua entry/profile cases passed
on arm64. C checks native paused flags and the maintenance pass; Lua checks
non-playing state and retained playback roots after collection. At cleanup both
resume and stop playback before device shutdown. This is an explicit test-game
policy, not automatic SDK audio focus handling.

Audio cases require a token-specific native acceptance marker after the audio
assertions. Lua also writes an exactly-once audio-close marker in its sandbox.
A successful graphics-only run cannot satisfy audio acceptance. Host payload
regressions check WAV/OGG bytes, the empty-WAV fixture and Lua module selection.

Foreground APK tests issue `KEYCODE_WAKEUP` and dismiss the emulator keyguard
before installation/launch. The controlled Home transition remains inside the
case. Preparation failures stop acceptance. If a case fails, the verifier also
collects Activity/window/power state and a best-effort `failure.png` before
uninstall. Evidence collection errors are logged without replacing the original
acceptance failure. This improves diagnostics for stalls without Lua errors or
native crashes; it does not add retries or change engine timing.

## Rust headless verification

Install the target standard library, then run the shared verification command:

```sh
rustup target add aarch64-linux-android
mise run test-android-rust -- --sdk "$ANDROID_SDK_ROOT" --serial emulator-5582
```

Use `x86_64-linux-android` for the Linux CI emulator. The command builds Debug
and Release C-only SDKs, relocates each SDK and runs the Rust core test suites
on the selected API 34 emulator. `--config Debug` selects one profile. Logs and
result metadata remain in `build-android-rust`. Each profile also rejects SDKs
with a previous ABI, a mismatched ABI or a wrong release. Those probes execute
on Android; an ABI mismatch must be rejected before calling the release query. Add `--assets` to build an asset-enabled SDK and run the shared PNG, atlas,
Aseprite and FPG suites. CI uses this option. The runner stages committed
`tests/fixtures` and `examples/aseprite` inputs in each test's private directory;
Rust tests select that root through `MALVADO_TEST_ROOT`. Both profiles also
build/run an external asset consumer from the relocated installed crates and
reject an asset-layout mismatch and a core-only SDK requested with assets.
The core-only SDK remains usable when the assets feature is disabled. They retain their normal
source-root default on desktop. Temporary files and staged assets are removed
with each invocation, including failed tests. Graphics/APK startup uses the separate NativeActivity fixture below. General APK game exports remain unavailable.

For direct Cargo use, set `MALVADO_SDK_PREFIX` to the installed target SDK,
`MALVADO_ANDROID_NDK` to NDK r27c and `MALVADO_ANDROID_RUNNER` to one executable
that accepts a target program path and returns its stdout and exit status.
The SDK uses this runner for ABI/release probes; it does not substitute host
layout values. The supplied runner requires `MALVADO_ANDROID_ADB` and
`MALVADO_ANDROID_SERIAL`. Set Cargo's target linker to the NDK API 28 clang
wrapper and Cargo's target runner to the same executable. The verification
script sets these values, including paths with spaces. General cross compilation
outside Android arm64/x86_64 remains unavailable.

The transport retries directory creation for at most 30 seconds when Android
reports exactly `fork failed: Out of memory` before the shell command can run.
Each attempt uses the remaining deadline. It records retries on stderr and
keeps ABI probe stdout unchanged. It does not retry permission/offline errors,
uploads, target program execution or failed tests. A target failure retains its
original status and output, and the owned directory is removed. Host regressions
cover recovery, deadline exhaustion, unrelated errors and program memory errors.
A live emulator check injected one preparation failure, then ran a native
scheduler test and verified successful directory cleanup.

## Rust NativeActivity startup

The installed graphical SDK also has a bounded Rust APK fixture:

```sh
mise run test-android-window -- --sdk "$ANDROID_SDK_ROOT" --serial emulator-5582 --language rust --installed-sdk --config Debug --work build-android/rust-window-Debug
```

Repeat with `--config Release` and a separate work directory. The command builds
and relocates the graphical C SDK, builds a Rust static library against its
installed crates and links a small NativeActivity entry adapter. Native graphics
ABI probes run on the selected emulator before linking. The Rust fixture checks
wrong-thread rejection, window creation, a red framebuffer pixel, process ticks,
exactly-once process cleanup, window closure and verified APK removal.

On Android, the window owner thread is the NativeActivity app thread. It is not
the Java UI thread or an arbitrary Rust worker. The installed helper compares
against the app thread before a window is created. Desktop thread rules remain
the same. This fixture does not deliver a general CLI Android game exporter.
Rust Back and same-process background/foreground checks use the same command
with `--lifecycle back` or `--lifecycle resume`. The latter requires native
pause/resume events, the same PID, a restored framebuffer pixel and before/resumed
screenshots, then Back cleanup. These fixtures do not establish process-death
recovery. Remaining Rust input/lifecycle/failure-path acceptance stays open;
the verifier rejects options for those unverified Rust APK modes. C/Lua fixture
coverage does not establish Rust behavior. Linux graphical Rust CI acceptance
remains pending until the new job passes.

Rust physical-key verification uses `--language rust --installed-sdk` with
`--lifecycle keyboard` or `--lifecycle keyboard-resume`. Rust checks A and Left
held/pressed/released snapshots, repeated non-consuming reads, neutral B and a
later neutral tick. The host requires Rust-specific completion markers before
injecting another key or Back. The resume case also retains PID continuity and
before/resumed/keyboard screenshots. Release while backgrounded and IME/text
input remain outside this check.

Rust primary-contact verification uses `--lifecycle touch`. Rust must observe
only the left mouse button, a contact near the logical center, positive
horizontal movement and release through its own API. The host waits for both
native and Rust release markers before Back, and retains before/released PNGs.
This establishes one contact through the existing mouse mapping; it does not
establish multi-touch, gestures or physical-device behavior.

Rust APK fixtures include a fixed binary/PNG/atlas payload. A test-only allowlist
copies it from the APK into the private app directory. Rust loads the atlas,
decodes the PNG through file and byte APIs, retains an image in a process and
checks that image owners release the window during shutdown. A binary snapshot
remains valid after unlink and after process/window cleanup. This uses installed
asset bindings and graphics from the same relocated SDK; it does not deliver a
general APK extraction API or CLI exporter.

Use `--language rust --installed-sdk --failure process` for the expected process
error case. The process owns an image resource and returns `Error::Process`.
The fixture requires the scheduler error, exactly-once destruction, an empty
process list, window release, a valid retained binary snapshot and uninstall.
The host requires separate error/resource-cleanup markers; successful rendering
is not the acceptance condition for this negative case. Lifecycle/audio options
cannot be combined with an expected failure.

Use `--failure missing-image` for the Rust PNG failure case. The packager omits
`red.png` while retaining the atlas and binary input. The native fixture verifies
the APK omission and absence of a staged file. Rust requires the image-load
error and propagates it through a process. Acceptance requires the scheduler
error and the same cleanup/snapshot checks as the process-error case. It does
not treat a crash or a missing completion marker as success.

Rust APK sound verification uses `--language rust --installed-sdk --audio`.
It loads packaged WAV media, checks invalid/empty paths and volume boundaries,
clones ownership, pause/resume/stop, bounded finite completion and final device
release. The native entry also requires a closed audio device before success.
This checks emulator API state, not audible output or audio-focus policy.

The Rust audio fixture also loads WAV and OGG streams through `Music`, checks
missing/empty media and invalid volumes, repeated play, pause state, resume/stop,
finite OGG completion and stream release before device shutdown. The host
requires a Rust-specific stream completion marker as well as the final audio
marker. Silent fixtures do not establish audible quality, latency or focus.

Add `--lifecycle resume` to the Rust `--audio` command to verify game-requested
pause across Home/foreground. Both WAV instances and WAV/OGG streams must stay
paused through frame maintenance, then resume/stop after foreground. A paused
sound belongs to a process resource; process shutdown must release it exactly
once before the final audio-device owner is dropped. Native device closure is
checked after Rust returns. The host also requires native pause/resume, the same
PID, restored pixels, before/resumed screenshots and uninstall. This establishes
explicit pause continuity, not automatic audio-focus behavior.

A repeated local Rust Debug audio/resume run exposed a raylib 5.5 shutdown race:
the AudioTrack callback accessed the mixer mutex after `CloseAudioDevice`
destroyed it. Normal API-state and earlier resume successes do not establish
reliable shutdown. The upstream fix is included in raylib 6.2. The current upgrade uses that
checksum-pinned release without editing third-party source. The updated C/Lua/Rust checks passed locally in both profiles. Linux/Windows
candidate CI remains open.

## Raylib 6.2 dependency update

The build pins the official 6.2 archive and its SHA-256. That release stops the
audio device before destroying the callback mutex and owns WAV decoder context
cleanup. The Malvado adapter removes its 5.5-only WAV free. Installed SDKs and
Rust link QuartzCore on macOS. Android final links retain NativeActivity and
raylib's `--wrap=fopen` requirement.

Raylib 6.2 resolves relative `fopen` reads from APK assets first, then the private
app directory/filesystem. Writes use the private directory with filesystem
fallback. This applies to final graphical APK links; headless SDKs do not add
the wrapper. Use absolute sandbox paths when a test or game requires a writable
file to take priority over a packaged asset with the same name. The Rust binary
snapshot fixture uses that rule to test removal of its actual source file.
This backend resolution rule does not deliver CLI APK/AAB exports or replace
the general packaged-game acceptance gate.

## Keyboard release while backgrounded

Use `--lifecycle keyboard-background` with the shared APK verifier. It checks
A and Left for C, both Lua entries and Rust. Android's bounded key-combination
command holds the tested key with Left Shift for four seconds. The host waits
for native and language-specific held markers, confirms Home/background while
the injection is still active, waits for key-up, then returns to the app.
Each return must preserve the PID and produce its own numbered resumed-pixel
marker. Release and neutral-tick confirmation must precede Back and cleanup.
Before, per-key resumed and final keyboard screenshots remain in the work folder.

The adapter queues releases on focus loss; it does not wait for a key-up that
Android can deliver to another Activity. A release may be observed before the
Activity is fully paused. Reads stay non-consuming and future presses work.
This covers physical A/Left state and does not add text/IME, multi-touch,
process-death recovery or an audio-focus policy.

## Contact cancellation while backgrounded

Use `--lifecycle touch-background` with C, either Lua entry or Rust. The host
injects a contact at the screen center, waits for each language to confirm it,
moves it horizontally and waits for movement confirmation. It then requires
confirmed background state before sending `UP`. On foreground, the PID must
match and a numbered restored-pixel marker must precede the resumed screenshot.
Each language must confirm release and remain neutral until Back cleanup.
Before/resumed/released PNGs and result metadata remain in the work directory.

The verifier always attempts to release its own injected contact on failure.
The adapter neutralizes buttons on cancel, pause, focus loss and surface loss.
A move cannot revive cancelled state; a fresh down is needed. This verifies one
primary contact through existing mouse queries. Multi-touch, physical mouse
hardware and process-death recovery remain outside this acceptance check.

Both profiles passed this check on clean `7c3a049` in
[Android CI run 44047602484](https://github.com/jepemo/malvado/actions/runs/44047602484).
Downloaded artifacts contain 46 distinct successful APK packages and 86 valid
PNG headers per profile. All four background-contact language/entry results
confirm release while backgrounded and the same PID after foreground.

## Destruction before the first native surface

The Android window adapter waits for a pending native window before it calls
raylib initialization. It processes earlier NativeActivity events with the
existing callbacks and leaves INIT_WINDOW for raylib. If DESTROY arrives first,
`mv_window_open` returns 0 without allocating a graphics context. Lua reports
`could not open window` and closes its runtime. Rust returns `Error::Platform`.
This uses the existing native event loop and game scheduler. It does not change
third-party source or implement runtime resource-overlay recovery.

Run the bounded cancellation fixture with the pinned Java toolchain:

```sh
mise run test-android-window -- --sdk "$ANDROID_HOME" --serial emulator-5580 \
  --installed-sdk --failure startup-destroy
mise run test-android-window -- --sdk "$ANDROID_HOME" --serial emulator-5580 \
  --language lua --entry declarative --failure startup-destroy
mise run test-android-window -- --sdk "$ANDROID_HOME" --serial emulator-5580 \
  --language rust --installed-sdk --failure startup-destroy
```

Use `--entry explicit` for the other Lua entry and `--config Release` for the
other profile. The test-only Java Activity calls `finish()` in `onCreate`, before
Android can create the first surface. The native checks require DESTROY with no
current or pending window, no graphics or audio device, and a language-specific
failed-open result. Lua also requires its runtime context to be released and its
process cleanup marker to remain absent. Each case retains its token, logs and
result JSON, and verifies APK removal. Screenshots cannot establish this path because it has
no application surface. Both CI profiles include all four language/entry cases.

Both cancelled-startup profiles passed on clean `d8a01fd` in
[Android CI run 44051919472](https://github.com/jepemo/malvado/actions/runs/44051919472).
Downloaded artifacts verify 50 distinct APKs, 86 valid PNG headers and all four
language/entry cancellations per profile. Every APK result retains all four SDK
and both runtime archive inputs. Debug also passed the native Rust probe path
that previously failed during directory preparation. The same source passed
native Windows and general macOS/Linux CI. This historical source did not
include the live resource-overlay checks described below.

## Live Activity recreation

Use `--lifecycle recreate` with C, either Lua entry or Rust. This mode uses a
test-only NativeActivity receiver. After the first framebuffer is verified, the
host requests `Activity.recreate()`. The old runtime must release its resources
before the new instance renders. Require the same PID, numbered cleanup and
ready markers for both generations, then Back cleanup and verified uninstall.
Before/recreated PNGs and per-case results remain in the work directory.

```sh
mise run test-android-window -- --sdk "$ANDROID_HOME" --serial emulator-5582 \
  --installed-sdk --lifecycle recreate
```

Both profiles passed C, both Lua entries and Rust on API 34 arm64. The adapter
queries the Activity's actual initial focus because Android can retain focus
without another focus-gained callback during recreation. It restores backend
focus before the event wait. JNI ownership and error paths have host regressions.
No third-party source or game scheduler changed. These checks do not establish
resource-overlay updates, process-death recovery, audible output or audio-focus
policy. The new recreation cases still need pinned x86_64 CI acceptance.

### C process-error cleanup

Use `--installed-sdk --failure process` with `test-android-window` to verify
C process failure inside an APK. The callback creates a process-owned GPU
texture and returns `MV_STEP_ERROR`. The fixture requires `MV_PROCESS_ERROR`,
zero live processes, and exactly one texture release and process destructor.
Context destruction must not repeat either cleanup. The test then closes the
window, checks graphics/audio device state and verifies APK removal.
Both Debug and Release run in the Android CI matrix. This checks native API
state and cleanup; it does not establish physical-device or audible behavior.

### C missing-image cleanup

Use `--installed-sdk --failure missing-image` to test a failed image load with
a live process-owned texture. The absent APK image must return `NULL`. The
callback then returns `MV_STEP_ERROR` through the same scheduler error path.
Require a distinct missing-image marker as well as the cleanup assertions
above. A generic startup failure cannot satisfy this check. Debug and Release
CI retain independent per-failure results and verify APK removal.

### Runtime resource-overlay relaunch

Use `--lifecycle overlay` with `test-android-window` to apply a real resource
update while the game is running. The fixture builds a separate runtime
resource-overlay APK. It targets only the unique acceptance package and changes
one explicitly overlayable application-label string. No system overlay changes.
Build Tools 34 `aapt2` compiles both resource tables. Both APKs use the fixture's
test-only signing key.

The host requires the overlay to be disabled before activation and enabled
afterwards. It does not send the manual recreation broadcast. Android must
clean the first native Activity and render the second in the same process.
The fixture verifies the native framebuffer, retains before/recreated PNGs,
closes the second generation with Back and verifies removal of both APKs.
The overlay is also removed on failure; game removal runs even if overlay
cleanup fails. This does not test arbitrary overlays, process-death recovery,
physical devices or audible output.


Resource-overlay relaunch passed C, both Lua entries and Rust in Debug/Release
on API 34 arm64. Eight unique game/overlay pairs retained sixteen PNGs and
same-process, two-generation cleanup evidence. Lua/Rust also checked packaged
assets and their runtime/scheduler cleanup after relaunch. Both x86_64 CI
profiles include these cases and retain separate overlay evidence; hosted
acceptance remains open. The 223 host headless checks pass.

### Forced process stop and cold restart

Use `--lifecycle restart` to stop a live game with `am force-stop`, then launch
the installed APK again. The host first requires native framebuffer readiness
and records the old PID and screenshot. It requires process disappearance after
the stop, a different live PID after launch, and new framebuffer evidence from
that PID. Logcat queries for restart readiness and final cleanup use `--pid`;
stale markers from the killed process cannot establish success. Logs are kept.

The second process must render, close with Back and pass ordinary language
resource/scheduler cleanup. Both screenshots and both PIDs remain in the result
artifacts. Uninstall is required even after a failed restart. A forced stop
cannot run game destructors, so this check makes no first-process destructor
claim. It does not establish low-memory Activity restoration or saved-game
state recovery. The Android fixtures still use test-only packaging and signing.

Cold-restart checks passed C, both Lua entries and Rust in Debug/Release on
API 34 arm64. Eight unique APK results retain sixteen screenshots, old/new
PIDs and final cleanup evidence. The four host restart regressions and all
224 headless checks pass. Both x86_64 CI profiles include the mode and retain
separate evidence; hosted acceptance remains open.

## Physical-key text input

Android key-down events now use the device's `KeyCharacterMap` and modifier
state to fill the existing Unicode text queue. C `mv_text_input_next`, Lua
`mv.text_input_next` and Rust `Window::text_input_next` share this behavior.
Key repeats enqueue characters; key-up does not. Controls, DEL, invalid scalars
and Android dead-key composition markers are discarded. JNI failures enqueue
nothing and release owned local references and thread attachments. Existing
JNI exceptions remain owned by their caller.

Host regressions cover printable ASCII, accented and non-BMP scalars, rejected
values and JNI failure paths. An exported installed-SDK C APK also passed
mixed-case `aZ0` input on API 34 arm64. IME composition, dead-key composition,
on-screen keyboard control and physical-device acceptance remain unavailable
or outside this alpha's emulator evidence.

Add `--text-input` to `mise run test-android-c-export`,
`mise run test-android-export` or `mise run test-android-rust-export` with the
same installed CLI/SDK, Android SDK, serial and work arguments as the export
checks above. This selects the bounded typing case instead of the export-error
matrix. It waits for red pixels, injects `aZ0` through Android and requires the
language's ordered text-completion marker, successful game exit, session cleanup
and uninstall. Rust also requires exactly-once process-resource destruction.
Release uses the existing Release SDK and disposable external signing-key flags.
Lua checks both explicit and declarative entries.

The full local text matrix passed on API 34 arm64: C, both Lua entries and Rust
in Debug and Release (eight APKs). Result JSON, runtime logs and red-pixel PNGs
are retained in `build-android/text-{c,lua,rust}-{Debug,Release}`. Release packages
were non-debuggable. The host standard suite passed all 227 checks. This is
emulator key-event evidence, not a physical keyboard or IME acceptance result.
Pinned x86_64 CI and exact-candidate acceptance remain open.

### Physical text in emulator fixtures

Pass `--lifecycle text` to `mise run test-android-window --` to inject
`aZ0` as keyboard events. Use the same option for C, both Lua entries and
Rust. The host checks the original process's language marker, captures a
screenshot and uses Back for normal shutdown. Both Android CI profiles run
this mode on the pinned x86_64 image. The eight local arm64 cases passed.
