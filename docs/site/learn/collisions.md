# Collisions

Move a circle into a square and show a message while they overlap. Once you
can see the overlap, change the circle's hit area and compare it to the
shape on screen.

## Run the example

Install the [desktop SDK](../../start/installation/) and download
`main.lua`. Open a terminal in the download directory.

```sh
malvado run main.lua
```

Move the blue circle with the arrow keys. The message appears when
the circle overlaps the orange square and disappears when they separate.
Press Escape or close the window to exit.

<!-- example -->

## Choose which shapes and processes to check

A graph describes how a process looks. A collider describes the geometry used
for collision. Here each primitive graph supplies its own collider. The
circle is centered on its process position and the square uses a top-left
pivot. Process position, angle and scale transform the selected geometry.
Camera and scroll offsets affect drawing only.

`collision(Square)` looks for an overlapping process made by the
`Square` constructor. Pass a specific live handle to check just that
instance. The result is the first matching handle or `nil` when no match
overlaps. Use `collisions` to collect all matches. Edges that only touch
have no overlap.

Try adding `self.collider = mv.collision.circle {radius = 8}` before the
circle's loop. The visible circle still has a radius of 20. Move its edge
into the square and watch how much overlap is needed before the message
appears. Assign `nil` to restore the graph-derived collider, or `false`
to disable it.

## Playground

<!-- playground -->

The [`rect_overlaps` reference](../../reference/lua/collision/rect_overlaps/)
covers the rectangle input and return value.

## Move against a wall

A process overlap query reports contact without moving a process. For a
level made from fixed rectangles, build a world with `world_create()` and
`world_add_rect`. Use `world_move(world, player, dx, dy)` to sweep the
player's selected collider along the requested step and apply the permitted
distance. Start outside obstacles. On a hit the move stops at contact.
Sliding and collision response between moving processes need gameplay code.

See the [Lua collision](../../reference/lua/collision/) and
[world](../../reference/lua/world/) references. The [collision
guide](../../concepts/collision/) covers transforms and C queries. The
[experimental Rust guide](../../concepts/rust-collision/) covers native
colliders and static worlds.
