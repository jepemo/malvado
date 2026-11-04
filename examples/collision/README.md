# Circle collision

Run this example from the repository root:

```sh
./build/malvado run examples/collision/main.lua
```

The circle and square each run in their own process. Use the arrow keys to move the circle. The message appears while the circle touches or overlaps the square. Press Escape to exit.

The example tests the distance from the circle center to the nearest point on the square. This detects contact at the corners as well as along the edges.
