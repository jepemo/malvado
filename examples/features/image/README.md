# Image

Load one PNG after opening the window. Draw it at its original size and
release its reference when the lesson ends.

[C](c/main.c) · [Lua](lua/main.lua) · [Rust](rust/main.rs)

Use the [shared SDK setup](../README.md#prepare-a-project) with
`examples/features/image` as the lesson source. Copy the shared asset to the
generated project root, then run the project with a display. Each variant
loads the PNG from the game directory:

```sh
cp "$source/assets/tile.png" lesson/tile.png
cd lesson
malvado run
```

The program prints `image: 32x32` and shows a red tile at `(32, 32)` in a
160 × 120 window. The window closes after about two seconds. You can close it
earlier with the window close control.

The red tile is original Malvado artwork. It and the example sources use the
project [Apache 2.0 license](../../../LICENSE).
