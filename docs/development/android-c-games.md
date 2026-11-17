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

The entry runs on the NativeActivity game thread. Close owned contexts,
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
