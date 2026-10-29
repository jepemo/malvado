# C processes without Lua

This headless example creates a C process, runs three ticks, and checks that the process is destroyed. After you build the project, run `build/example-c-processes`. Configure with `-DMALVADO_BUILD_LUA=OFF -DMALVADO_BUILD_RAYLIB=OFF` to build without Lua or raylib.

Run the equivalent Lua example with `build/malvado-run --headless examples/c/processes/main.lua`. Both examples print the same three-tick count.
