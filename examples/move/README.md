# Move

A first interactive scene: one process draws an orange circle and moves it
with the arrow keys. The process uses `self.delta` to keep its speed in
pixels per second when its activation cadence changes.

Build Malvado, then run:

```sh
./build/malvado run examples/move/main.lua
```

## Controls

- Arrow keys: move the circle.
- Escape: end the process and close the game.

The example needs a display or a virtual display. It uses no assets.

## Languages

This example currently has a Lua version only. Aligned C and Rust
versions are incomplete work tracked for the teaching catalog in
[examples](../README.md).
