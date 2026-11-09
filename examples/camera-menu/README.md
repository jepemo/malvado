# Clipped screen-space inventory

Run from the repository root:

```sh
./build/malvado run examples/camera-menu/main.lua
```

Use Left and Right to scroll the colored inventory tiles. The region and its
outline stay fixed. The inventory uses `self.camera = false`, a screen-space
`region`, and its own `scroll`. Neither the process position nor its collision
geometry moves. The purple world marker uses the main camera at (200, 100).
Press Escape to exit. See [camera composition](../../docs/api/cameras.md).
