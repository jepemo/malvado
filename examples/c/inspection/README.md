# Inspect C processes

This headless example creates an owner, one owned child, and one independent
child. It compares ownership with creation history and captures process and
lineage snapshots. The records are copies; they do not keep processes alive.

```sh
cmake -S . -B build-c -DMALVADO_BUILD_LUA=OFF -DMALVADO_BUILD_RAYLIB=OFF
cmake --build build-c
./build-c/example-c-inspection
```

The output reports three processes, one owned child, and two creation children.
Combined captures return `MV_BUFFER_TOO_SMALL` and the required counts when
the buffers are too small. This example demonstrates the zero-capacity query;
a real game can use the counts to allocate buffers. See the [C contract](../../../docs/api/c-contract.md#inspect-a-snapshot)
and the [Lua inspection example](../../inspection/README.md).
