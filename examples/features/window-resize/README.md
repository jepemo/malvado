# Runtime window resize

Resize an open window and read its dimensions in logical pixels.

[C](c/main.c) · [Lua](lua/main.lua) · [Rust](rust/main.rs)

Use the [shared SDK setup](../README.md#prepare-a-project) with
`examples/features/window-resize` as the lesson source. Use a graphical
desktop GLFW build and a display. Enter the generated project and run:

```sh
cd lesson
malvado run
```

The window opens at 320 × 240, requests 640 × 360 after about one second,
then closes after another second. The program prints the initial and
effective dimensions. The window system can constrain the requested size.
Use the close control to exit early.

There are no assets. The sources use the project
[Apache 2.0 license](../../../LICENSE).
