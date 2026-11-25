# First Rust games

- [Hello World](hello.rs) opens a window and redraws text on each activation.
- [Moving balls](moving-balls.rs) spawns three copies of one async function.
  Each ball keeps its own speed, direction, and position across frame waits.

Use the repository's Rust toolchain, an installed Malvado SDK with graphics,
and a display. From the repository root, run:

```sh
export MALVADO_SDK_PREFIX="/path/to/installed sdk"
cargo run --manifest-path examples/rust/Cargo.toml --bin hello
cargo run --manifest-path examples/rust/Cargo.toml --bin moving-balls
```

Hello World shows a white greeting. The cyan, orange, and green balls move at
different speeds and bounce at the window edges. Both windows close on Escape
or when you use the window's close control. Neither example needs assets.

For a game with input, collision, and scoring, see [Catch!](../catch/README.md).
For C and Lua equivalents of the basic gameplay patterns, see the
[aligned lessons](../features/README.md). These Rust sources use the project
[Apache 2.0 license](../../LICENSE).
