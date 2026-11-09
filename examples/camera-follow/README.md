# Camera following

Run from the repository root:

```sh
./build/malvado run examples/camera-follow/main.lua
```

Move the blue player with the arrow keys. The main camera keeps the player
centered while the orange wall moves across the view. The instructions stay in
screen space because the HUD uses `self.camera = false`. Press Escape to exit.

`camera_follow(self)` uses the existing engine update; no controller process is
needed. The wall is visual geometry, not a physical obstacle. See
[cameras](../../docs/api/cameras.md) for manual controls, explicit views and lifetime.
