# Graphics and sprites

Show the same sprite twice and rotate one copy. Both actors use the same
picture, but each process controls its own position and angle.

## Run the example

Install the [desktop SDK](../../start/installation/) and extract the ZIP into
an empty directory. Run the command from that directory. Keep
`assets/sheet.json` and `assets/sheet.png` side by side.

```sh
malvado run sprites.lua
```

Two large sprites appear. The one on the right is rotated 45 degrees.
Press Escape or close the window to exit. The ZIP includes the Apache 2.0
license and credits for the original Malvado atlas assets.

<!-- example -->

## Choose a graphic and an origin

A standalone PNG can be loaded with `mv.load_asset("player.png")` and
assigned directly to a process's `graph`. Its origin defaults to the top
left. Pass `{pivot = "center"}` as the loader's second argument if you
want it to rotate about its center.

A sprite is a graphic selected from a library. A sprite sheet puts several
graphics in one image. The atlas metadata describes where to find each
graphic and can record names, pivots, trim offsets and animation frames.
Load the atlas JSON with `load_asset`, then select an exact name like
`"one"` or an integer ID with `graph(sheet, selector)`.

The pivot is the point that lands on the process's x and y. Rotation and
scale use that point. The example's pivot is at (4, 4) in the original
8 × 8 canvas. The atlas uses the trim offset to place the cropped pixels
relative to that pivot. The processes scale by 12 to make the tiny sprite
easy to see. Change the right actor's angle or set both scales to 1 and
compare the result.

The selected graphic retains its library and image. Load one sheet and
reuse its graphics. Reloading a sheet for each actor does extra work.
Keep the metadata and PNG together when you move the project.

See the [Lua graphics selector reference](../../reference/lua/graphics/graph/),
the [image and atlas guide](../../concepts/images/) and the
[experimental Rust atlas guide](../../concepts/rust-atlas/). Continue with
[animation](animation.md).
