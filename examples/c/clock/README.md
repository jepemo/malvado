# First C example: logical clock

This headless example uses the initial C API. It does not require Lua or raylib. It advances the logical clock by 1/30 second at 60 Hz, then runs the scheduler for each of the two steps. Advancing the clock alone does not run processes.

```sh
cmake -S . -B build-c -DMALVADO_BUILD_LUA=OFF -DMALVADO_BUILD_RAYLIB=OFF
cmake --build build-c
ctest --test-dir build-c --output-on-failure
./build-c/example-c-clock
```

The example checks that the process runs twice, and that no time was dropped. See the [process example](../processes/README.md) for scheduler use.
