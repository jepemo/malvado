# Catch!

A small complete game in C, Lua and Rust. A cyan player rectangle moves along
the bottom of the window. An orange ball falls. Catch the ball with the player
to score one point. The ball respawns above the screen at a new horizontal
position. A miss respawns the ball without scoring.

The same game runs in the three languages and uses the process model: the
player and the ball are independent processes over the shared core scheduler.
Each version uses the engine to draw the process graphics and to test the ball
against the player.

## Entry points

- [C](c/main.c)
- [Lua](lua/main.lua)
- [Rust](../../bindings/rust/malvado/examples/catch.rs)

## Window and behavior

- Window: 640 × 360, titled `Catch!`.
- Player: cyan rectangle, 80 × 16, centered at `(320, 320)`.
- Ball: orange circle, radius 12, starting at `(80, 0)`.
- Score: white text at `(20, 20)`, size 24, starting at zero.

## Controls

- Left and right arrow keys: move the player.
- Escape: end the game.
- The window close control also ends the game.

## Rules

- The player moves at 400 pixels per second and its center is clamped to
  `[40, 600]`.
- The ball falls at 180 pixels per second.
- On a catch, the score increases by one and the ball respawns above the
  screen. On a miss, the ball respawns without increasing the score.
- Each respawn moves the ball 160 pixels to the right, wrapped by the window
  width.
- The game uses primitive graphics and built-in colors. It uses no assets.

## Run

Build Malvado, then run the Lua version with a display:

```sh
./build/malvado run examples/catch/lua/main.lua
```

Build and run the C version:

```sh
mise run build
./build/example-c-catch
```

The Rust version needs an installed graphics SDK. Set `MALVADO_SDK_PREFIX` and
run the example with the `graphics` feature:

```sh
MALVADO_SDK_PREFIX="/path/to/installed sdk" \
  cargo run --manifest-path bindings/rust/Cargo.toml -p malvado \
  --features graphics --example catch
```

The sources use the project [Apache 2.0 license](../../LICENSE).
