# Native animation playback

Build and run from the repository root:

```sh
mise run build-c
./build-c/example-c-animation
```

A native process advances an immutable two-frame sequence on the shared
scheduler. Graph IDs are logical IDs; this headless example does not load or
draw assets. At ticks 6 and 12 it verifies the frame boundary and loop.

The caller owns the definition and player state. It destroys the context before
releasing the definition, so no process can use freed timing data. See
[automatic animation](../../../docs/api/animations.md) for native/Lua contracts.
