# World collision

Run from the repository root:

```sh
./build/malvado run examples/world-collision/main.lua
```

Use the arrow keys to move the circle. The static wall stops it. The circle graphic supplies its collider automatically. Press Escape
to exit. The wall is drawn separately from its collision geometry.

The player keeps the world in `self.world` and calls `world_move(dx, dy)`.
Use `world:move(player, dx, dy)` when an explicit process is useful.
