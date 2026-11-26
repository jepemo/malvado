# Input and movement

Move a circle by holding the arrow keys. Tap Space to bring it back to
the center, or click to leave a marker where the pointer was.

## Run the example

Install the [desktop SDK](../../start/installation/) and download
`movement.lua`. Open a terminal in the download directory.

```sh
malvado run movement.lua
```

The circle stays inside the window, even if you keep holding a key at an
edge. A left click places a yellow marker. Press Escape or close the window
to exit.

<!-- example -->

## Choose held state or a transition

`mv.key(mv.KEY_RIGHT)` stays true while you hold the key. That lets the
process move every turn. `mv.key_pressed(mv.KEY_SPACE)` is true on the
logical tick that receives the press. Use that for a single action and
`mv.key_released` for the release. All readers see the same tick's edges.
A process that skips that tick can miss an edge. Use an active input process
and send messages if a slower process needs to receive those actions.

Multiply the speed in pixels per second by `self.delta`, the
[time since the last activation](frames-and-time.md). The code normalizes the
direction to keep diagonal movement at the same speed. It also clamps the
circle's center to the window bounds minus its radius. Clamping the center
to the edge itself would let half the circle leave the view.

## Read a mouse or gamepad

`mv.mouse_position()` returns logical window coordinates. This scene uses the
default view at (0, 0), so the example can draw the marker at those coordinates.
A moving camera needs a [coordinate conversion](cameras-and-scrolling.md).
Use `mv.mouse_button` for a held button, or `mv.mouse_pressed` and
`mv.mouse_released` for edges with a constant such as `mv.MOUSE_LEFT`.

On supported backends, check slots 0 through 3 with `mv.pad_connected(slot)`.
Use `mv.pad_button(slot, mv.PAD_A)` for held state and `mv.pad_pressed` or
`mv.pad_released` for edges. A left stick's horizontal value from
`mv.pad_axis(slot, mv.PAD_LEFT_X)` is between −1 and 1. For stick movement,
ignore a small range around zero to avoid drift.

Try switching the Space check from `key_pressed` to `key`. Hold Space
and an arrow key together. The circle now resets on every turn while Space
is held.

See the [Lua input reference](../../reference/lua/input/), the [shared input
tick rules](../../concepts/keyboard/) and the [experimental Rust input
guide](../../concepts/rust-input/).
