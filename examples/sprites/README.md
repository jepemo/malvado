# Sprites

This example animates a cat from six PNG images. The process assigns one loaded image to `self.graph` and changes it 15 times per second. Press **Escape** to exit.

Build Malvado, then run this command from the repository root:

```sh
./build/malvado run examples/sprites/main.lua
```

The installed CLI also runs the example with `malvado run .` from this
directory or with `malvado run /path/to/main.lua` from any directory.

The frames in `cat/` are from **Shepardskin**’s
[Cat Sprites](https://lpc.opengameart.org/content/cat-sprites) and use
[CC0 1.0](https://creativecommons.org/publicdomain/zero/1.0/). The PNG frames
include extra background padding. See the [image guide](../../docs/api/images.md)
for spritesheets and atlases.
