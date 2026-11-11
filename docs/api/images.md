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

`draw_image` coordinates must be integers that fit a C `int`. Out-of-range
values raise an error without adding a draw command.

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

C programs can also create an image from packed RGBA bytes with
`mv_image_from_rgba(width, height, pixels, length)`. It requires an open
window, dimensions in 1–8192 and at least `width * height * 4` bytes. The
input is borrowed during the call; the caller owns the returned image and must
destroy it before closing the window. It returns NULL on invalid input or
failure. It is not available in headless mode.

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

All fields are required except `pivot_x` and `pivot_y`, which default to zero. IDs must be positive and unique. Names must be unique. Regions must fit in the image. Names and image paths must be valid UTF-8 without ASCII control characters. Names have at most 127 bytes and image paths have at most 511 bytes. Lookups compare bytes exactly and do not normalize Unicode. It supports up to 4096 sprites and 1 MiB of JSON. It rejects unknown fields, duplicate keys, escaped zero bytes, and unescaped control characters in JSON strings. Outside strings, only JSON whitespace is accepted. Numeric fields must be exact integers, including when written with a decimal point or exponent. A literal backslash followed by `u0000` is allowed. This schema is separate from the Aseprite subset described below.

Load the PNG and JSON in your `start` function. Use the image named by the metadata; `load_atlas` checks its dimensions but does not check its filename. Paths are relative to the working directory:

```lua
local image = mv.load_image("sprites.png")
local file = assert(io.open("atlas.json", "rb"))
local json = file:read("a")
file:close()
local atlas = mv.load_atlas(image, json)
```

Inside a process, call `mv.draw_sprite(atlas, 10, 120, 80, 2, 90)` before `frame`. The last two arguments are optional: scale defaults to one and rotation to zero. The atlas keeps its image alive while it is in use. An unknown ID or invalid metadata raises an error.

For C, include `malvado/atlas.h` and link `malvado_assets`. Pass the JSON bytes and image dimensions to `mv_atlas_parse`. Use `mv_atlas_find` to get a sprite's region and pivot, then draw it with `mv_image_draw_region`. The atlas owns its metadata; destroy it with `mv_atlas_destroy`. The image path from `mv_atlas_image` and the name in a copied `MvAtlasSprite` are borrowed and remain valid until the atlas is destroyed. Load the image yourself. `mv_atlas_count(NULL)` returns zero and `mv_atlas_image(NULL)` returns NULL. `mv_atlas_find` and `mv_atlas_find_name` return 0 for invalid pointers or an unknown sprite and leave the output unchanged. See the runnable [C atlas example](../../examples/c/atlas/README.md). The C parser does not need a window. To build it without Lua or raylib, configure with `-DMALVADO_BUILD_ASSETS=ON` and provide cJSON. The default C-only build keeps this optional library disabled.

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


## Use images and atlas sprites as process graphics

An image returned by `load_image` can be assigned directly to `self.graph`.
Its position is the top-left corner, as with `draw_image`. Automatic image
drawing starts after the process first activates. The process's `x`,
`y`, `angle`, and `size` transform the image, and its full dimensions provide
the default rectangular collider. Process defaults may share an image. The
process holds the image alive while its `graph` refers to it. Images still
require an open window and cannot be loaded in headless mode.

For an atlas sprite, create a fixed descriptor with
`mv.graphics.sprite(atlas, id)` and assign it to `self.graph`. It uses the
sprite's region and metadata pivot for drawing and its rectangular collider.
It keeps the atlas and its image alive. An unknown ID raises an error. The
function is available when atlas support is built. A whole atlas is not a graph.
For animation, call `animation_frame` and replace `self.graph` with a
descriptor for the selected ID; the descriptor does not advance on its own.
Pixel-perfect collision is not supported.

```lua
local sprite = mv.graphics.sprite(atlas, 42)
local Actor = mv.process(function(self)
    while true do mv.frame() end
end, {defaults = {graph = sprite}})
```

The legacy `frame()` draws its own image graph with its historical centered
pivot. That command is not drawn again by the automatic image graph path. The automatic
collider also uses a centered pivot while the process uses legacy `frame()`.
Legacy FPG sheets require an explicit collider.

## Import an Aseprite JSON array

`mv.load_aseprite(image, json)` loads a restricted Aseprite JSON array export into an atlas. Load the PNG with `mv.load_image` first. The function retains the image just like `mv.load_atlas`; release Lua references when no longer needed. It raises an error for invalid or unsupported input.

```lua
local image = mv.load_image("sheet.png")
local file = assert(io.open("sheet.json", "rb"))
local json = file:read("a")
file:close()
local atlas = mv.load_aseprite(image, json)
local id = mv.animation_frame(atlas, "default", elapsed_ms)
-- Call mv.draw_sprite(atlas, id, x, y) inside a process.
```

The JSON must contain an array of 1–4096 frames, a `meta.image` path, and a `meta.size` object. The declared dimensions must match the loaded PNG. Frame names must be unique, valid UTF-8 strings without ASCII control characters, with at most 127 bytes; image paths must be valid UTF-8 without ASCII control characters, with at most 511 bytes. Name lookup compares UTF-8 bytes exactly and does not normalize Unicode. Each frame must be unrotated. For a trimmed frame, `spriteSourceSize` gives the crop offset and dimensions within `sourceSize`; the crop must fit the original canvas and match the atlas region. For an untrimmed frame the offset must be zero and the dimensions must match. Durations are integer milliseconds from 1 to 3,600,000. Frames receive IDs 1 through N in file order. Without a pivot slice, their origin is the top-left of the original canvas; the atlas uses a negative crop offset as its region pivot so drawing and rotations preserve that origin. Their durations form the `default` animation. The PNG pixels are not changed.

The importer accepts only scale `"1"` and, when present, format `"RGBA8888"`. Forward, reverse, `pingpong`, and `pingpong_reverse` frame tags become named animations. For example, frames 1, 2, 3 play as 1, 2, 3, 2 in `pingpong` and 3, 2, 1, 2 in `pingpong_reverse`. The cycle does not repeat an endpoint; one- and two-frame tags are supported. Each occurrence keeps that frame’s duration. As with other animations, non-looping playback holds the last frame in this sequence. Their zero-based `from` and `to` bounds are inclusive and must refer to exported frames in order. Tag names must be unique, valid UTF-8 strings without ASCII control characters, with at most 127 bytes, and cannot be `default`. The atlas supports at most 255 tags and 4096 animation frames in total, counting the default animation and each tag. Playback uses `mv.animation_frame` and its `loop` argument; explicit tag repeat counts are preserved as metadata but are not applied automatically. In C, `mv_atlas_animation_repeats(atlas, name, &count)` copies an explicitly supplied count (0–65535), returning 0 and leaving the output unchanged when it is absent or the tag is unknown. `mv.animation_repeats(atlas, name)` returns that count or `nil` if it is absent or the animation does not exist; names containing a zero byte raise an error. Aseprite stores this field as a decimal string; the importer rejects invalid or out-of-range values. Optional tag colors in `#RRGGBB` or `#RRGGBBAA` form are preserved. `mv.animation_color(atlas, name)` returns a new `{r, g, b, a}` table with channels from 0 to 255, or `nil` when the tag has no color or does not exist. A name with a zero byte raises an error. In C, `mv_atlas_animation_color(atlas, name, &color)` copies the color, or returns 0 without changing the output. A missing alpha defaults to 255.

A single slice named `pivot` can set a custom origin. Keys must have strictly increasing frame indexes within the exported frames. Before the first key, the origin is the top-left of the original canvas. Each key applies until the next key, and its bounds must fit the original canvas of every frame it covers. A key without a `pivot` point or with a zero width or height uses the default origin until the next key; zero-sized slice bounds indicate a hidden slice. A present point is relative to the slice; the importer adds the slice offset and subtracts each frame’s crop offset to obtain its atlas-region pivot. Optional slice colors use the same hexadecimal formats as tag colors. In C, `mv_atlas_pivot_color(atlas, &color)` returns that color or 0 without modifying the output when absent. In Lua, `mv.atlas_pivot_color(atlas)` returns a fresh `{r, g, b, a}` table, or `nil` if no color is present. The imported pivot applies to both `draw_sprite` and process graphics, including their automatic colliders.

Escaped zero bytes, unescaped control characters in JSON strings, non-JSON whitespace, other extra tag fields, nonempty layers, other slices, rotated frames, unknown fields, duplicate keys, invalid bounds, and metadata over 1 MiB are rejected. It does not import multiple slices, nine-slice centers, layers, or palette data. The automatic process collider covers the cropped rectangle, not the full original canvas or an alpha mask. It does not verify that `meta.image` names the supplied image. Use a suitable export without these features. The error identifies JSON-hash exports, unsupported pixel formats or scales, rotated frames, unsupported tags, multiple slices or pivot slices, and nonempty layers; other invalid or unsupported metadata uses a general error.

C programs can call `mv_atlas_parse_aseprite(json, length, width, height, &error)` without Lua or a graphics backend when asset support is enabled. The caller owns the returned atlas and destroys it with `mv_atlas_destroy`; the error, when provided, is a static string. `mv_atlas_find` returns `source_width`, `source_height`, `trim_x`, and `trim_y` in `MvAtlasSprite`. For the native atlas schema, source size equals the region and the trim offset is zero. Lua sprite drawing uses the crop offset. `mv.sprite_info(atlas, id)` returns a fresh table with `id`, `name`, region `x`, `y`, `width`, `height`, `pivot_x`, `pivot_y`, `source_width`, `source_height`, `trim_x`, and `trim_y`. Unknown IDs and invalid arguments raise an error. Modifying the table does not change the atlas. `mv.sprite_id(atlas, name)` looks up an exact, case-sensitive name and returns its ID or `nil`; names containing a zero byte raise an error. In C, `mv_atlas_find_name(atlas, name, &sprite)` copies the matching sprite and leaves the output unchanged on failure. The [Coric fixture](../../tests/fixtures/aseprite/coric/README.md) is an unmodified Aseprite export. The other test fixtures and the crop-and-pivot example are original Malvado data, not exports from Aseprite. See the [acceptance decision](../specs/assets.md#alpha6-aseprite-acceptance-decision) for their coverage and limits.


## Load an asset by path

After `import_globals()`, use `load_asset(path, options?)`. The explicit module
function is `mv.load_asset`. Both return the same managed image or library:

```lua
require("malvado").import_globals()
start(function()
  local icon = load_asset("icon.png")
  local hero = load_asset("hero.json")
  local enemies = load_asset("enemies.fpg")
end)
```

The loader recognizes PNG signatures, the supported 8-bit FPG signature, native
atlas schemas 1/2, Aseprite JSON-array exports, and version 1 converted-FPG
manifests. It checks JSON structure and then calls the existing native importer
for full validation. A `.json` suffix alone does not select a format. Unknown,
ambiguous, malformed and unsupported inputs raise errors. This does not support
native `.aseprite` files, folders as libraries, or mounted ZIP packages.

The optional `format` is `"png"`, `"fpg"`, `"atlas"`, `"aseprite"`, or
`"fpg-manifest"`. It selects the importer, which still validates the data.
For PNG process graphics, set `pivot = "center"` to rotate and collide around
the image center. The default is `"top-left"`. See [PNG pivots](#shared-png-textures-and-automatic-pivots).

Unknown options and format names raise errors. `transparent_zero` is an optional
boolean only for FPG input, defaulting to true for loaded library graphics.
Converted FPG PNGs retain the transparency already encoded by the converter.
Existing `fpg:image(id, transparent_zero)` calls keep their explicit policy.

Paths are relative to the working directory. Referenced atlas/manifest images
are relative to their metadata file, or use their explicit absolute path.
Sources must be readable, seekable files of at most 64 MiB; the native parser's
smaller metadata limits still apply. Asset-enabled builds use the shared
[native source and path service](asset-sources.md) for reading and routing.
Detection is a hint; the selected importer still validates the complete format.
An empty path, zero byte in a path, missing
file, read failure or changed source during reading raises an error.

Load visual assets during initialization after the window opens. With an open
window, libraries load their images and validate actual image dimensions.
Without a window, PNG loading fails; FPG and JSON libraries provide metadata
without loading images. Native-atlas bounds are then checked without a texture
size, and Aseprite uses its declared canvas dimensions. An atlas or manifest loaded this way does not later acquire textures
automatically. Use a new visual load when a window is available. FPG libraries
retain pixels and can convert them when a later explicit graph selection needs
an image.

Returned libraries retain the complete supported native metadata and existing
lookup methods/functions: IDs, names/raw FPG bytes, palette/gamma, control
points, pivots, original/cropped sizes and animation timing. No metadata is
converted to a reduced Lua table. Lua references retain libraries and their
images; garbage collection and the existing process resource rules release
them. The low-level loaders and parsers remain available for advanced use.

## Select a static graphic

`graph()` selects one static graphic. It does not advance an animation:

```lua
require("malvado").import_globals()
local hero
local Player = process(function(self)
  self.file = hero
  self.graph = graph(42)
  while true do frame() end
end)
start(function()
  hero = load_asset("hero.json")
  Player({x = 100, y = 100})
end)
```

Use `graph(id)` inside the running process, `graph(library, id)` explicitly,
or `library:graph(id)`. The explicit namespace is
`mv.graphics.graph(library, id)`; `mv.graph` is the same function. The selector
is available with asset support. It accepts atlases (including Aseprite), FPGs
and converted-FPG manifests. IDs must be positive integers present in the
library. Invalid IDs/types raise errors. A standalone image is assigned directly
to `self.graph`, without calling `graph()` or setting `self.file`.

The implicit call uses only the scheduled coroutine's `self.file`. Missing file
context or calls outside that coroutine raise errors. Explicit calls work
without a running process and ignore its contextual file. No shared default
library is stored globally. Both game entry forms use these same rules.

The result is immutable and reusable. It retains its original library and image.
Changing `self.file` affects later lookups only. Two processes can share one
descriptor, including when their file fields differ. Clearing/replacing a graph
releases its references when no other user retains it. A retained process handle
can retain its fields under the existing process lifetime rules.

Atlas graphs keep the existing trim and pivot calculation. FPG and converted-FPG
graphs use defined control point 0 as the pivot, or top-left when that point is
absent/undefined. The automatic collider uses those same bounds/pivots. Explicit
colliders and `collider=false` keep their established behavior.

Visual loads reuse the library's images. Graph selection from a low-level parsed
FPG can convert and cache its pixels when a window is open; index 0 is transparent
by default. Loaded FPGs use the loader's `transparent_zero` option. A manifest
parsed from JSON alone has no images; load its path with `load_asset` for visual
use. Headless selection creates static geometry without textures. An atlas or
manifest loaded without images must be loaded again for visual use; graph lookup
reports this limitation when a window is open. A descriptor never loads a new
image during rendering.

Modern selection uses `self.file` and `self.graph`. `self.fpg` and
`self.fpgIndex` retain meaning only in the explicit legacy compatibility profile.
The existing `mv.graphics.sprite(atlas, id)` remains available. See [automatic animation](animations.md) for `animation()` playback. The advanced
`animation_frame` sampler remains available for explicit time/frame selection.

## Shared PNG textures and automatic pivots

Repeated PNG loads share the native texture while it is alive. This includes
`load_asset`, `load_image`, and atlas/manifest image loading. Lua calls can return
different managed objects with their own drawing metadata; they retain shared
native image references. Ending the process that loaded one reference does not
invalidate another process's graph. The cache itself does not pin the texture.
Existing process/context resource ownership can retain references until cleanup.

The key is the normalized absolute file path (with symlink resolution on POSIX),
not file content. Equivalent relative/absolute paths share that key. A live
texture is a snapshot; changing the file does not reload it. Release its last
native reference before loading a changed version. RGBA-created images are not
file-cached. This does not add a hot-reload or asset-packaging system.

By default, an image assigned to `self.graph` keeps its top-left origin. Select a
centered pivot explicitly for rotating actors:

```lua
self.graph = load_asset("ship.png", {pivot = "center"})
self.size = 0.1
self.angle = 45
```

PNG `pivot` accepts `"center"` or `"top-left"`. Other values and non-PNG pivot
options raise errors. The center is exactly half the source dimensions, including
half-pixel centers for odd dimensions. Automatic drawing and collider inference
use the same pivot, rotation and scale. Pivot options belong to the Lua image
reference; two references to the same texture can use different pivots.
Explicit `draw_image` coordinates remain top-left; `draw_image_region` uses the
pivot supplied in its arguments. Other asset formats retain their metadata pivots.

Automatic PNG colliders still cover the full rectangular source image. Transparency
or an outlined graphic does not imply pixel-mask collision. For gameplay, set an
explicit source-space circle/rectangle. The engine applies `self.size` and angle
once, including the collider offset. Do not scale the collider a second time.
See [Space Asteroids](../../examples/space_asteroids/) for shared centered sprites
with explicit source-space geometric approximations.

In C, each successful `mv_image_load` returns one owned reference, which can have
the same pointer as an earlier load. Call `mv_image_destroy` once per successful
load, before window close. One release leaves other references valid; the last
release unloads and removes the cached image. C callers continue to provide
pivots explicitly through `MvImageTransform`.

## PNG bytes in memory

`mv_image_from_png(data, length)` decodes PNG bytes with the native backend.
It needs an open window on the owner thread. The input is borrowed for the
call only. The returned image owns an independent texture and has no path
cache entry. Release it with `mv_image_destroy` before closing the window.

The input limit is 64 MiB. Image dimensions must be in `1..8192`. The native
loader checks the signature and dimensions before decoding. The backend
validates the encoded pixels; this API does not promise strict PNG archive or
checksum validation. Invalid input, missing graphics support and backend or
allocation failure return NULL. No file is opened.

Rust `Image::from_png(window, bytes)` uses this operation. It retains the window
and reports native failure as `Error::Platform`, like other image constructors.
The input bytes can be changed or released after success.

## Native image references

`mv_image_retain(image)` acquires one more reference to a live image, including
an uncached RGBA or PNG-buffer image. It returns 1 on success. NULL or reference
overflow returns 0 without changing the image. Destroy each acquired reference
once with `mv_image_destroy`. Retention does not copy pixels or retain the
window. Keep the window open until every reference is released, and use these
operations on its owner thread. Rust `Image::clone` already shares its safe
owner and needs no change.
