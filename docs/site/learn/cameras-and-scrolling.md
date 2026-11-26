# Cameras and scrolling

Follow a player across a world that extends beyond the window. The
player stays near the center while a wall moves across the view and
the instructions stay fixed.

## Run the example

Install the [desktop SDK](../../start/installation/) and download
`main.lua`. Open a terminal in the download directory.

```sh
malvado run main.lua
```

Move with the arrow keys. Watch the wall slide left as the player moves
right. Press Escape or close the window to exit.

<!-- example -->

## Separate the world from the window

The player and wall have coordinates in the world. `camera_follow(self)`
moves the main camera so the player's origin stays in the center of the view.
A process with `self.camera = false` draws in screen space. The instructions
use that setting and a higher `z` to draw over the other processes.

You can set the main camera's world position with `camera_set(x, y)`.
For a smaller view, use `camera_create {viewport = region {x = 40, y = 60,
width = 300, height = 240}}` and assign the camera to the processes it
should show. The viewport is the screen rectangle that clips their drawing.
A process can apply a further clip with `self.region` or an offset with
`self.scroll = scroll(dx, dy)`. Processes across views share z and creation
order. Persistent text draws after them in screen space.

## Convert a point

For the current translation-only camera, subtract the camera position and
the process's local scroll from a world point, then add the viewport origin.
For example, world (120, 65), camera (100, 50), scroll (5, 5) and viewport
origin (40, 60) give screen (55, 70).

Reverse those operations to map a mouse position back to world
coordinates for that view. Check whether the pointer is inside the
viewport before using it to pick an object. Lua cameras currently have
no zoom setting. Changing a process's `size` scales its own graphic and
collider.

Try removing `camera_follow(self)` from the example. The camera stays
at its initial location and the player can walk out of view. Its world
movement works as before.

See the [Lua camera reference](../../reference/lua/cameras/), the
[camera guide for C and Lua](../../concepts/cameras/) and the
[experimental Rust camera guide](../../concepts/rust-camera-animation/).
