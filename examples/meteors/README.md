# Meteors

A first look at independent processes. Three meteor processes each keep their
own position and speed, move down the screen, and wrap to the top when they
leave the bottom. There is no central entity list and no manual update loop: the
scheduler runs each process, and the engine draws each process graphic.

Build Malvado, then run:

```sh
./build/malvado run examples/meteors/main.lua
```

## Controls

- Escape: end the processes and close the game.

The example needs a display or a virtual display. It uses no assets.

## Languages

This example currently has a Lua version only. Aligned C and Rust
versions are incomplete work tracked for the teaching catalog in
[examples](../README.md).
