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
restoration, C/Rust payloads or release signing.

See the [CLI specification](../specs/cli.md) for host export commands and
current limits.
