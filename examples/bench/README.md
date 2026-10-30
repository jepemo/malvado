# Process benchmark

Hold **Space** to add animated processes. Press **Escape** to stop them and close the window.

Run this command from the repository root after you build Malvado:

```sh
./build/malvado-run examples/bench/main.lua
```

The display shows the process count and the wall time between controller ticks. This interval includes the frame limit, scheduling, and rendering. It is not a separate measurement of CPU or render time. The random seed is fixed. Use the same machine, window size, process count, and display settings when you compare runs.

The example requires a display or virtual display. The [historical example](https://github.com/jepemo/malvado/blob/4670d8a/examples/bench/main.lua) used LÖVE and global functions.
