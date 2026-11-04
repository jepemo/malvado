# Circle collision

Run this example from the repository root:

```sh
./build/malvado run examples/collision/main.lua
```

The circle and square each run in their own process. Use the arrow keys to move the circle. The message appears while the circle overlaps the square. Press Escape to exit.

The circle and square use process graphics. `process.collision` checks their colliders, including the corners. Touching without overlap is not a collision.
