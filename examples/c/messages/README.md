# Send a message in C

This headless example sends bytes to a process before its first tick. The queue copies the bytes, so the sender can change its buffer. The process receives the message and releases its copy.

```sh
cmake -S . -B build-c -DMALVADO_BUILD_LUA=OFF -DMALVADO_BUILD_RAYLIB=OFF
cmake --build build-c
./build-c/example-c-messages
```

The program prints `Received ping`. See the [message contract](../../../docs/api/c-contract.md#exchange-messages) for queue limits, errors, and opaque message ownership.
