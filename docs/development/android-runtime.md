# Android Lua runtime package

The Android SDK can now deliver a reusable Lua player. It uses NativeActivity
and the existing Lua runtime and process scheduler. It does not contain a test
game or a fixture-specific asset list.

Build the arm64 player with the pinned NDK and install only its runtime component:

```sh
cmake -S . -B build-android-runtime \
  -DCMAKE_TOOLCHAIN_FILE="$ANDROID_SDK_ROOT/ndk/27.2.12479018/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-28 \
  -DCMAKE_BUILD_TYPE=Debug -DMALVADO_BUILD_LUA=ON \
  -DMALVADO_BUILD_RAYLIB=ON -DMALVADO_BUILD_ASSETS=ON
cmake --build build-android-runtime --target malvado-android-game --parallel
cmake --install build-android-runtime --component AndroidRuntime \
  --prefix "$PWD/android-runtime"
```

The component contains `libexec/malvado/android/arm64-v8a/libgame.so`, SDK version
metadata and dependency notices. It needs no Android CLI executable. The host
exporter packages this prebuilt library; it does not compile the engine.

The player reads the exporter-generated asset index. It accepts only ordinary
relative paths, creates an exclusive private directory and copies all indexed
files there. Lua modules, standard file reads and native asset services can use
the same project paths. The APK contains the original immutable asset files too.
Each invocation runs `main.lua`, with either supported Lua entry form. It then
returns to application storage and removes its temporary game directory without
following symbolic links. Runtime errors and the exit status use the
`MalvadoGame` logcat tag. Stderr redirection is restored after each run.

The player owns the process working directory during execution. Files written
inside the temporary game directory are temporary too; this is not a saved-game
storage API. A forced process death can leave that directory until app data is
removed. This first runtime package does not promise persistent saves, state
restoration or C/Rust payloads.

See the [CLI specification](cli.md) for host export commands and
current limits.

Verify the installed host CLI and the component-only target runtime with an
explicit API 34 arm64 emulator:

```sh
mise run test-android-export -- \
  --cli /absolute/path/host-sdk/bin/malvado \
  --runtime-sdk "$PWD/android-runtime" --android-sdk "$ANDROID_SDK_ROOT" \
  --serial emulator-5582 --work build-android-export
```

The verifier exports and executes both Lua entries and a process-error game.
It checks nested binary assets, atlas/PNG loading, actual screenshot pixels,
notices, exact error diagnostics, session cleanup and uninstall. It retains
logs, screenshots and a result report. Missing capabilities remain listed in
the version checklist; the default command checks APKs. Add the AAB flags described in
[app bundle checks](android-bundles.md) to verify bundles separately. It does not
certify production signing.


For release verification, build and install the same component with
`-DCMAKE_BUILD_TYPE=Release` to a separate prefix. The component now installs
`share/malvado/android-runtime.txt` with the language, build profile and ABI.
Release export rejects a missing or non-Release profile; rebuild and reinstall
older runtime prefixes before using it.

Add `--release` to `test-android-export` and point `--runtime-sdk` at that Release
prefix. The verifier creates a disposable keystore outside each game project
and removes it even on failure. It verifies unsigned APKs separately, then signs,
installs and runs both Lua entries and a process-error game. Compiled versions,
assets, pixels, non-debuggable state, runtime cleanup markers and uninstall are
required. Release APKs reject `run-as`; their session cleanup is checked through
the runtime's actual cleanup result. Debug checks also inspect private storage.
