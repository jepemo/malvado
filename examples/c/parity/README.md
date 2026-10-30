# Compare C and Lua processes

These examples run the same three-tick scenario. A controller sends a message, freezes and wakes a worker, then stops it. Both programs print the same trace. CTest compares their output.

```sh
./build/example-c-parity
./build/malvado-run --headless examples/c/parity/main.lua
```

The C example also runs when Lua and raylib are disabled. Its process state is explicit. Lua keeps its coroutine state across calls to `frame()`.
