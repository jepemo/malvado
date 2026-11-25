# Android C game packages

A C game can now use the installed Android graphical SDK without Lua. It uses
the same private asset staging and diagnostics as the Lua player. It does not
add a scheduler. The game creates its context and drives the existing C process
scheduler, as on the desktop.

Find the Android SDK and create the shared game target:

```cmake
cmake_minimum_required(VERSION 3.20)
project(game C)
find_package(Malvado CONFIG REQUIRED COMPONENTS graphics)
if(ANDROID)
  malvado_add_android_game(game main.c)
else()
  add_executable(game main.c)
  target_link_libraries(game PRIVATE Malvado::graphics)
endif()
```

`malvado_add_android_game(TARGET SOURCES...)` is available only with the Android
`graphics` component. It creates a shared-library target named `TARGET`, with
output `libgame.so`. It accepts one ordinary C `int main(void)` entry point;
a different signature fails compilation. Add include directories, definitions,
source files and libraries through the usual CMake target commands. Find and
link optional components, such as `Malvado::assets`, when the game uses them.
The helper captures the installed runner path when the SDK is found, so later
package discovery does not change the runner location.

The entry runs on the NativeActivity game thread. It can run again after
Activity recreation; C static data persists within the process. Reset per-run
state. Close owned contexts,
resources and the window before returning. Return zero for success and nonzero
for failure. The runner logs stderr and the return status with `MalvadoGame`.
A session-cleanup failure forces status 1. Project paths are relative to a private
temporary game directory. The asset index is the same one used by Lua exports.
The runner removes the session on an ordinary return, without following links.
Writes in that directory are temporary; process death and saved-game recovery
have the limits described for the [shared runtime](android-runtime.md).

Use a full installed C Android SDK. A Lua-only `AndroidRuntime` component does
not contain the C libraries and CMake package needed for a native project.
The SDK can be built with `MALVADO_BUILD_LUA=OFF`. Use Debug or Release SDK
libraries to match the game build. This path does not establish a Rust game
entry or C++ support.

## Export from the installed CLI

New `malvado new --lang c PATH` projects now include the Android CMake branch above.
For an existing project, replace its unconditional `add_executable(game ...)`
with that branch. The export target must be named `game`.

Build and install a full target SDK, for example:

```sh
cmake -S . -B build-android-c \
  -DCMAKE_TOOLCHAIN_FILE="$ANDROID_SDK_ROOT/ndk/27.2.12479018/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-28 \
  -DCMAKE_BUILD_TYPE=Debug -DMALVADO_BUILD_LUA=OFF \
  -DMALVADO_BUILD_RAYLIB=ON -DMALVADO_BUILD_ASSETS=ON
cmake --build build-android-c --parallel
cmake --install build-android-c --prefix "$PWD/android-c-sdk"
```

From the C project directory, use the same APK flags as Lua:

```sh
malvado export --target android-arm64 --format apk \
  --runtime-sdk /absolute/path/android-c-sdk \
  --android-sdk "$ANDROID_SDK_ROOT" --output ../dist/game.apk \
  --id org.example.cgame
```

The exporter detects C when the project has `CMakeLists.txt` and no `main.lua`.
It requires CMake and pinned NDK r27c. By default, it uses the NDK inside the
selected Android SDK; `--ndk PATH` selects another installed r27c prefix. It
compiles and links the game in an owned staging directory and validates the
arm64 shared library. It does not execute the game during export. A failed
configure, compile, link or dependency check removes staging and publishes no
APK. Sources ending in `.c`/`.h` and `CMakeLists.txt` are excluded from assets.
Nested binary files and required notices remain included.

Use a full Release target SDK with `--release`; an SDK containing only Debug
libraries is rejected. Manifest, unsigned output and external signing options
are the same as [Lua APK export](cli.md#android-apk-export-lua).
The current path supports statically linked SDK/game dependencies and the
engine's Android system libraries. NDK LLVM inspection rejects other shared
libraries instead of silently omitting them from the APK. Bundling additional
shared libraries, C++ and other Android export ABIs remain unavailable. Rust
has its own [production entry](android-rust-games.md).

Run `mise run test-android-c-export` with `--cli`, `--runtime-sdk`,
`--android-sdk`, `--serial` and `--work` to verify the complete path on arm64.
For `--release`, also supply an external disposable `--keystore`, `--key-alias`
and `--store-password-env`. The verifier creates ordinary CLI projects, checks
process-owned texture cleanup and actual pixels, executes expected process
errors and rejects compiler/entry-signature errors. Logs and results remain in
the work directory. It verifies removal of every installed APK.
