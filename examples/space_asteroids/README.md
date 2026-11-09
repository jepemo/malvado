# Space Asteroids

Turn with **Left** and **Right**, move with **Up**, and press **Space** to fire.
Holding Space fires once; release it before the next shot. A large asteroid
splits into three smaller asteroids. Destroy all 20 large/small targets to win.
If an asteroid hits your ship, the round ends. Press **Escape** to exit.

Build Malvado and run from the repository root:

```sh
./build/malvado run examples/space_asteroids/main.lua
```

Each rock, shot, ship and star has its own process. `load_asset()` shares the
live native PNG texture across processes. `pivot = "center"` keeps rotation and
position centered without changing the default origin used by other examples.

The geometric colliders use source pixels, with process `size` applied once:
rocks use radius 96 (28.8 at size 0.3, 14.4 at size 0.15), shots radius 3, and the
ship radius 110 with offset_y 10 (radius 11 and offset 1 at size 0.1). These are
simple gameplay approximations; they do not test the image's individual pixels.
The engine rotates the ship collider offset with the sprite.

Fragments are independent of their creating shot so they remain after it hits.
The game explicitly removes all remaining rocks before ending its process tree.
Owned shots end with the ship; shots also stop when they leave the screen or
reach their lifetime. The result stays visible until Escape.

The example needs a display or virtual display. The [image/pivot contract](../../docs/api/images.md#shared-png-textures-and-automatic-pivots)
and [collision guide](../../docs/api/collision.md) describe resource lifetime,
automatic bounds and explicit overrides. The [original example](https://github.com/jepemo/malvado/blob/4670d8a/examples/space_asteroids/main.lua)
used LÖVE and global functions.
