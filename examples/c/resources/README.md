# Retain a process resource in C

This headless example gives a resource to a process and retains it for later use. When the process ends, the retained reference keeps the data available. Releasing that reference calls the destructor once. The example uses stack data so the destructor records cleanup; real heap data must be freed by its destructor.

```sh
cmake -S . -B build-c -DMALVADO_BUILD_LUA=OFF -DMALVADO_BUILD_RAYLIB=OFF
cmake --build build-c
./build-c/example-c-resources
```

See the [resource contract](../../../docs/api/c-contract.md#owned-resources) for transfer, errors, and context shutdown.
