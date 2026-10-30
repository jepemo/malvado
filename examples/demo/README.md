# Malvado demo

Choose a section with the arrow keys and **Enter**, or click an option. Press **Escape** to return to the menu, then press it again to exit.

- **Sprites:** shows two animated images.
- **Keyboard:** displays typed text.
- **Texts:** moves text across the screen.
- **Fade:** press **Space** to fade the screen out and in.

Build Malvado and run this command from the repository root:

```sh
./build/malvado-run examples/demo/main.lua
```

For an automated graphics check, set `MALVADO_SMOKE=1`. The demo visits all four sections and exits. It requires a display or virtual display. The [original demo](https://github.com/jepemo/malvado/blob/4670d8a/examples/demo/main.lua) used LÖVE and global functions.
