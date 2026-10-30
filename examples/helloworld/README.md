# Hello World

This example starts a process and displays a greeting. Press **Escape** to close the window.

Build Malvado, then run:

```sh
./build/malvado-run examples/helloworld/main.lua
```

For an automated graphics check, set `MALVADO_SMOKE=1`. The example closes after three ticks. It requires a display or virtual display.

This version uses the Malvado 2 Lua API. The [original example](https://github.com/jepemo/malvado/blob/4670d8a/examples/helloworld/main.lua) used LÖVE and global functions.
