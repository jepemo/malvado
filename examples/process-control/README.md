# Control Lua processes

This headless example creates owned and independent workers. It queries both
relations, changes priority, sends signals, and keeps handles after termination.
It also shows how a creation-tree signal includes an independent worker.

After building Malvado, run:

```sh
./build/malvado run --headless examples/process-control/main.lua
```

See the [process guide](../../docs/api/lua-processes.md) for process lifetime,
errors, and the flat and namespace forms of these functions.
