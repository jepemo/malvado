# Android Rust game packages

Rust games can now use the same installed Android session runner as C and Lua.
They keep the safe `Game` API and the existing native process scheduler. The
Android entry is a small bridge; it does not install another event loop.

Add a static library target to a normal Cargo package:

```toml
[lib]
name = "game"
path = "android.rs"
crate-type = ["staticlib"]
```

Keep the ordinary `fn main()` in `main.rs`. Add this sibling `android.rs`:

```rust
#![cfg(target_os = "android")]
include!("main.rs");
malvado::android_entry!(main);
```

Use a relative include path that matches your project layout. Source files used
with `include!` must be valid in that context; move crate-level attributes to the
bridge when required. Invoke the macro once in the Android library. The bridge
is excluded on desktop targets, which keep the ordinary binary entry.

The entry takes no arguments. It accepts `()` or another Rust `Termination`
return type, such as `Result<(), Error>`. Success maps to native status 0. Other
termination values, returned errors and unwinding panics map to 1. The macro
contains unwinding panics before returning through C. A `panic=abort` profile
still aborts the process and cannot provide ordinary cleanup evidence.

Release owned games, resources and windows before the entry returns. Normal
Rust drop behavior also applies during unwinding. The shared runner logs stderr
and status, stages ordinary assets into a private session and removes that
session on return. Static state persists within the process. Persistent saves
and process-death restoration have the [shared runtime](android-runtime.md)
limits.

For SDK consumers that build manually, find the installed Android `graphics`
component and call `malvado_add_android_rust_game(game ARCHIVE_PATH)` after Cargo
produces the Android static library. The function links the archive with the
shared runner and SDK graphics target, producing `libgame.so`. It rejects a
missing archive. Rebuild and reinstall older SDK prefixes to get this helper
and `android_entry!`.

The actual Debug and Release arm64 game checks include native pixels, binary
assets, process-owned image cleanup, a returned process error and a contained
panic. Rust host tests also call the C entry for success, error and panic and
verify exactly-once drop behavior. This is production entry support, not a
claim of full alpha.8 acceptance on every host.

## Installed CLI export

New `malvado new --lang rust PATH` projects include the library target and bridge.
Existing projects can add the two small changes above. Use one staticlib target
in the package at the project root, with a direct non-optional `malvado` SDK
dependency. This ensures the existing ABI-checking build script runs. The
exporter asks Cargo for that target;
it does not copy or rewrite your game sources.

Rust's existing SDK verifier executes native ABI probes on the target. Export
therefore requires an executable probe runner through `--probe-runner PATH` or
`MALVADO_ANDROID_RUNNER`. Keep the runner outside the project assets. Missing
runners are errors, not skipped checks. The
runner must print probe stdout and return its exit status. For the supplied
repository ADB runner, create a small executable wrapper:

```sh
#!/bin/sh
exec python3 /absolute/path/malvado/tools/android_rust_runner.py "$@"
```

Set `MALVADO_ANDROID_ADB` to the Android SDK's absolute `platform-tools/adb` path
and `MALVADO_ANDROID_SERIAL` to the selected emulator serial, such as
`emulator-5582`. The ADB runner uses bounded commands and an owned remote probe
directory, retains diagnostics and removes that directory after each probe.
It is an authoring verifier; no probe runner is included in the APK.

From the project directory:

```sh
malvado export --target android-arm64 --format apk \
  --runtime-sdk /absolute/path/full-android-sdk \
  --android-sdk "$ANDROID_SDK_ROOT" --probe-runner /absolute/path/probe-runner \
  --output ../dist/game.apk --id org.example.rustgame
```

The host needs Cargo, CMake, NDK r27c and the installed Rust target
`aarch64-linux-android`. The SDK version must match the host client. Add
`--release` and a full Release SDK for a release APK. Use the existing external
keystore/password-environment options for signing, or leave release output
unsigned. The same manifest flags apply. No compiler or Cargo is needed by the
installed game.

Cargo outputs and the C runner link stay in owned export staging. Cargo can
create or update the project's ordinary lockfile. Rust/C source files, Cargo
manifests/lockfiles and `target` folders are excluded from APK assets. Normal
nested files and notices remain included. Missing staticlib/entry symbols,
compiler/link failures, ABI rejection and unsupported shared dependencies fail
without publishing an APK. This first path does not promise arbitrary Cargo
workspaces or other Android export ABIs. Release AAB uses the same build path;
see [bundle generation and checks](android-bundles.md).

Use `mise run test-android-rust-export` with `--cli`, `--runtime-sdk`,
`--android-sdk`, `--serial` and `--work` for bounded installed-CLI verification.
For `--release`, also supply a disposable external keystore, alias and password
environment variable as in the C verifier. This tests normal execution, a
process error, a contained panic and a rejected compiler error, with actual
pixels, native assets, exactly-once drop markers, session cleanup and uninstall.
