# C processes without Lua

This headless example creates a C process, runs three ticks, and checks that the process is destroyed. After you build the project, run `build/example-c-processes`. Configure with `-DMALVADO_BUILD_LUA=OFF -DMALVADO_BUILD_RAYLIB=OFF` to build without Lua or raylib.

Run the equivalent Lua example with `build/malvado-run --headless examples/c/processes/main.lua`. Both examples print the same three-tick count.

To use the installed C core in another CMake project, install the build to a directory of your choice:

```sh
cmake --install build-c --prefix /path/to/malvado-sdk
```

In your project's `CMakeLists.txt`, use:

```cmake
find_package(Malvado CONFIG REQUIRED)
target_link_libraries(my_game PRIVATE Malvado::core)
```

When you configure the game, pass `-DCMAKE_PREFIX_PATH=/path/to/malvado-sdk`. This initial target provides the headless C core. Graphics and asset SDK targets are not available yet.
