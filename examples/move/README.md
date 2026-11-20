# Move

A first interactive scene. Three balls bounce around the screen on their
own, and one player circle follows the arrow keys. Every moving thing is
its own process, which shows the process model that Malvado is built on.
A screen process starts the balls and the player as its children, so they
all end together when you press Escape.

Build Malvado, then run:

```sh
./build/malvado run examples/move/main.lua
```

## Controls

- Arrow keys: move the orange player circle.
- Space: hold to move faster.
- Escape: end every process and close the game.

The example needs a display or a virtual display. It uses no assets.

## Languages

This example currently has a Lua version only. Aligned C and Rust
versions are incomplete work tracked for the teaching catalog in
[examples](../README.md).
