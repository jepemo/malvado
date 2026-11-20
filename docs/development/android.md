# Android verification

The [platform matrix](platforms.md) describes the supported SDKs,
arm64 game packages and verification limits. Use a dedicated API 34
emulator. The NDK is pinned to r27c (`27.2.12479018`) with API 28 as
the native compilation baseline.

## Local commands

On Linux x86_64, install the pinned SDK and emulator archives:

```sh
python3 tools/android_sdk.py --output build-android/sdk
python3 tools/android_archives.py --output build-android/pinned
abi=x86_64
```

On macOS arm64, use the matching archives:

```sh
python3 tools/android_sdk.py --host macos --output build-android/sdk
python3 tools/android_archives.py --host macos --output build-android/pinned
abi=arm64-v8a
```

Both installers verify hashes and require new output directories. Use the
Java version pinned in `mise.toml` for APK signing.

From the repository root, link the pinned image, create a dedicated AVD
and start its emulator:

```sh
export ANDROID_SDK_ROOT="$PWD/build-android/sdk"
export ANDROID_HOME="$ANDROID_SDK_ROOT"
ln -s "$PWD/build-android/pinned/emulator" "$ANDROID_SDK_ROOT/emulator"
mkdir -p "$ANDROID_SDK_ROOT/system-images/android-34/google_apis"
ln -s "$PWD/build-android/pinned/$abi" "$ANDROID_SDK_ROOT/system-images/android-34/google_apis/$abi"

python3 tools/android_avd.py --output build-android/isolated-avd \
  --image "build-android/pinned/$abi"
ANDROID_AVD_HOME="$PWD/build-android/isolated-avd" \
  "$ANDROID_SDK_ROOT/emulator/emulator" -avd malvado-alpha8 -port 5580 \
  -no-window -no-audio -no-boot-anim -no-snapshot \
  -gpu swiftshader_indirect -cores 2 -memory 2048
```

Keep the emulator running. In another terminal at the repository root,
select the same SDK, wait for boot and run the headless checks:

```sh
export ANDROID_SDK_ROOT="$PWD/build-android/sdk"
python3 tools/android_boot.py --adb "$ANDROID_SDK_ROOT/platform-tools/adb" \
  --serial emulator-5580 --timeout 240
mise run test-android -- --sdk "$ANDROID_SDK_ROOT" --serial emulator-5580
```

Add `--config Debug` or `--config Release` to select one profile. The
verifier requires an explicit serial and a booted API 34 emulator.

## Run the APK fixtures

Use `mise run test-android-window` with the same SDK and emulator to
verify graphics, input, assets and Activity lifecycle. For example:

```sh
mise run test-android-window -- --sdk "$ANDROID_SDK_ROOT" --serial emulator-5580 \
  --installed-sdk --config Debug --lifecycle touch
```

Select C, Lua or Rust with `--language`. Lua also accepts
`--entry explicit` or `--entry declarative`. Use a profile-specific
work directory when retaining logs, result JSON and screenshots.

For headless Rust, add the Rust target for the emulator, then run
`mise run test-android-rust` with the same SDK and serial. Use
`aarch64-linux-android` for arm64 or `x86_64-linux-android` for x86_64.
Add `--assets` to include asset checks.

## Android keyboard verification

Use `--lifecycle keyboard` for held keys and press/release transitions,
`--lifecycle keyboard-resume` for Home and foreground, or
`--lifecycle text` for the shared text queue. The
[production package suite](ci.md#android-production-packages) also tests
explicit keyboard show/hide and committed Unicode through an emulator IME.
See the [keyboard contract](../api/keyboard.md#android-on-screen-keyboard).

## Android audio API verification

Add `--audio` to the APK verifier to check resource ownership, playback
state and cleanup. Add `--lifecycle resume` to test explicitly paused
audio through Home and foreground. The emulator uses silent fixtures.
Audible playback remains a physical-device check.

After the checks, stop the dedicated emulator with
`"$ANDROID_SDK_ROOT/platform-tools/adb" -s emulator-5580 emu kill`.

## CI and evidence

`android.yml` runs independent Debug and Release fixture jobs and
[production package checks](ci.md#android-production-packages). It keeps
logs and results for startup, process errors, audio, input, Activity
recreation, overlay relaunch and cold restart.
