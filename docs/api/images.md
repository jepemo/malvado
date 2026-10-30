# Draw a PNG image

You can load a PNG image after you open a window. The image uses its original size. Its position is the top-left corner, in window pixels.

## Lua

Load the image in the function you pass to `start`. Draw it inside a process, before each `frame`:

```lua
local mv = require("malvado")
local image

local picture = mv.process(function()
    while not mv.key("escape") do
        mv.draw_image(image, 40, 40)
        mv.frame()
    end
end)

mv.start(function()
    image = mv.load_image("picture.png")
    local width, height = mv.image_size(image)
    print("Image size:", width, height)
    picture()
end)
```

Paths are relative to the current working directory. `load_image` reports an error if the file is not a valid PNG or there is no window. Images are not available in headless mode. Lua keeps an image alive while a draw command uses it. It releases the image when it is no longer used or when the runner closes.

Each Lua process has a `self.z` draw order, initially zero. Set it to a finite number to draw that process in front of lower values. Equal values use process creation order, so later processes draw on top. Changes take effect on the next render. This order applies to process draw commands; persistent text is drawn afterward. It does not change scheduler priority. In C, issue drawing calls in the order you want between `mv_window_begin` and `mv_window_end`.

## C

Include `malvado/platform.h` and link the raylib platform library. Open a window before loading an image. Check the result, and destroy the image before you close the window:

```c
MvImage *image = mv_image_load("picture.png");
if (image) {
    mv_window_begin((MvColor){0, 0, 0, 255});
    mv_image_draw(image, 40, 40);
    mv_window_end();
    mv_image_destroy(image);
}
```

`mv_image_load` returns `NULL` if loading fails or the window is not open. `mv_image_width` and `mv_image_height` return the dimensions in pixels, or zero for `NULL`. `mv_image_destroy(NULL)` is safe. Use the window and image functions on the window thread.

## Draw part of an image

Use a region to draw one sprite from a PNG sheet. The region is a rectangle in source pixels. The position is in window pixels. The pivot is relative to the top-left corner of the region. Scale is uniform. Rotation is clockwise in degrees.

```lua
-- Draw the 16 by 24 region at (32, 0), with its center at (120, 80).
mv.draw_image_region(image, 120, 80, 32, 0, 16, 24, 8, 12, 2, 90)
```

The last four arguments are optional: pivot x and y default to zero, scale to one, and rotation to zero. Call this function inside a process before `frame`. It reports an error for an invalid region or transform. The region must be inside the image. Scale must be positive; all transform values must be finite.

In C, pass an `MvImageRegion` and an `MvImageTransform` to `mv_image_draw_region` between `mv_window_begin` and `mv_window_end`. It returns 1 on success or 0 for invalid arguments. Use `mv_image_region_valid` to check the same arguments without drawing.

```c
MvImageRegion region = {32, 0, 16, 24};
MvImageTransform transform = {120, 80, 8, 12, 2, 90};
if (!mv_image_draw_region(image, region, transform)) {
    /* Handle an invalid region or transform. */
}
```

## Use an atlas

An atlas gives each region a stable numeric ID. Put the metadata in a JSON file:

```json
{
  "schema": 1,
  "kind": "graphics-library",
  "image": "sprites.png",
  "sprites": [
    {"id": 10, "name": "ship", "x": 0, "y": 0, "width": 16, "height": 24, "pivot_x": 8, "pivot_y": 12}
  ]
}
```

All fields are required except `pivot_x` and `pivot_y`, which default to zero. IDs must be positive and unique. Names must be unique. Regions must fit in the image. The schema accepts printable ASCII names and image paths. It supports up to 4096 sprites and 1 MiB of JSON. It rejects unknown fields and duplicate keys. Aseprite JSON is not supported yet.

Load the PNG and JSON in your `start` function. Use the image named by the metadata; `load_atlas` checks its dimensions but does not check its filename. Paths are relative to the working directory:

```lua
local image = mv.load_image("sprites.png")
local file = assert(io.open("atlas.json", "rb"))
local json = file:read("a")
file:close()
local atlas = mv.load_atlas(image, json)
```

Inside a process, call `mv.draw_sprite(atlas, 10, 120, 80, 2, 90)` before `frame`. The last two arguments are optional: scale defaults to one and rotation to zero. The atlas keeps its image alive while it is in use. An unknown ID or invalid metadata raises an error.

For C, include `malvado/atlas.h` and link `malvado_assets`. Pass the JSON bytes and image dimensions to `mv_atlas_parse`. Use `mv_atlas_find` to get a sprite's region and pivot, then draw it with `mv_image_draw_region`. The atlas owns its metadata; destroy it with `mv_atlas_destroy`. Its image path is available through `mv_atlas_image`; load that file yourself. The C parser does not need a window. To build it without Lua or raylib, configure with `-DMALVADO_BUILD_ASSETS=ON` and provide cJSON. The default C-only build keeps this optional library disabled.

## Animate an atlas

Schema 2 adds named animation sequences. Each frame refers to a sprite ID and gives its duration in milliseconds. For example, add this array to a schema 2 atlas that defines sprites 7 and 42:

```json
"animations": [
  {"name": "flash", "frames": [
    {"sprite": 7, "duration_ms": 100},
    {"sprite": 42, "duration_ms": 200}
  ]}
]
```

Schema 2 requires at least one animation. Names must be unique. Each animation needs at least one frame; the atlas can contain up to 256 animations and 4096 frames in total. Durations are integers from 1 to 3,600,000 ms. Unknown fields, duplicate names, and unknown sprite IDs are errors. Schema 1 remains unchanged.

Use `mv.animation_frame(atlas, "flash", elapsed_ms, loop)` to find the current sprite ID, then pass it to `mv.draw_sprite`. The elapsed time is a non-negative integer. `loop` defaults to `true`. If it is `false`, the last frame remains selected after the animation ends. A missing name or invalid argument raises an error. The caller controls time and playback state.

In C, `mv_atlas_animation_frame(atlas, name, elapsed_ms, loop, &id)` returns 1 on success. It returns 0 without changing `id` for an invalid argument or unknown animation. `loop` is 0 or 1. The atlas owns the sequence data.
