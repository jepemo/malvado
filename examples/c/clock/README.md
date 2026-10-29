# First C example: logical clock

This headless example uses the initial C API. It does not require Lua or raylib. It shows two ticks when the clock advances by 1/30 second at 60 Hz.

```sh
cmake -S . -B build-c -DMALVADO_BUILD_LUA=OFF -DMALVADO_BUILD_RAYLIB=OFF
cmake --build build-c
ctest --test-dir build-c --output-on-failure
./build-c/example-c-clock
```

This example tests the context and clock. See the [process example](../processes/README.md) for scheduler use.
