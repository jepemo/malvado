# Declarative game

This example returns the game settings and an initializer from `main.lua`.
The initializer starts one process. The game exits when that process ends.

From the repository root, run:

```sh
./build/malvado run --headless examples/declarative/main.lua
```

The output is `Hello from a process`. Remove `--headless` to open a window.
