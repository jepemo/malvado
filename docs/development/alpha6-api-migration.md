# Alpha.6 API migration

Alpha.6 keeps the shared C scheduler and both Lua entry forms. The minimal Lua
API reduces common calls while keeping both module and global access.
C keeps typed values, handles, ownership and explicit operations.

## Minimal Lua

```lua
require("malvado").import_globals()
local hero, world
local Player = process(function(self)
  self.file, self.world = hero, world
  self.graph = animation("bounce")
  self.size = 16
  camera_follow(self)
  while not key("escape") do
    if key("right") then world_move(3, 0) end
    frame()
  end
end)
start(function()
  hero = load_asset("examples/aseprite/sheet.json")
  world = world_create()
  Player({x = 100, y = 100})
end)
```

Run the snippet from the repository root to use the included Aseprite asset.
For more runnable programs, use [Aseprite](../../examples/aseprite/),
[FPG](../../examples/fpg/) and [camera following](../../examples/camera-follow/).
The [clipped inventory](../../examples/camera-menu/) uses returned `game.init`
instead of `start`; both forms accept the same globals and process behavior.

## Explicit Lua

```lua
local mv = require("malvado")
-- In initialization, after a window is available:
local hero = mv.load_asset("hero.json")
local world = mv.world_create()
local still = mv.graph(hero, 7)
local walk = mv.animation(hero, "walk")
local camera = mv.camera_create {x = 100, y = 50}
-- Inside game logic, with an existing player:
player.graph = walk
mv.world_move(world, player, 3, 0)
mv.camera_follow(camera, player)
local hit = mv.process.collision(player, Enemy)
```

See the [namespace inventory](../api/lua-namespaces.md) for functions
organized under `mv.graphics`, `mv.process`, `mv.collision` and other tables.

## Replace common calls

| Existing code | Minimal call | Contract |
| --- | --- | --- |
| Read a file, load its image and call the atlas/Aseprite parser | `load_asset(path, options?)` | [Images and formats](../api/images.md#load-an-asset-by-path) |
| `draw_image(image, x, y)` | `draw_image {image = image, x = x, y = y}` | The full image uses the top-left pivot by default |
| `draw_image_region(image, x, y, sx, sy, sw, sh, px, py, scale, rotation)` | `draw_image {image = image, x = x, y = y, source = {x = sx, y = sy, width = sw, height = sh}, pivot = {x = px, y = py}, scale = scale, angle = rotation}` | Use a source-relative pivot or `"center"` |
| `draw_sprite(atlas, id, x, y, scale, rotation)` | `draw_sprite {atlas = atlas, id = id, x = x, y = y, scale = scale, angle = rotation}` | Uses the sprite's atlas pivot and trim data |
| `mv.graphics.sprite(atlas, id)` | `graph(atlas, id)` or `graph(id)` using `self.file` | Static selection retains its original source |
| Sample a sequence each tick and assign a static sprite | `self.graph = animation("walk")` | [Independent automatic playback](../api/animations.md) |
| `mv.graphics.rect/circle/region/scroll` | `rect/circle/region/scroll` | Same constructors, validation and defaults |
| `mv.process.collision(self, Enemy)` | `collision(Enemy)` | Same canonical result and query ordering |
| Use a process only to update a view offset | `camera_follow(player)` | Following runs in the existing tick |

Implicit file/world/query calls work only in the actual scheduled process,
including when globals are imported. Nested user coroutines do not borrow the
caller's process. Use explicit arguments outside that context. Changing
`self.file` does not change a resolved graph or animation. Unknown options and
invalid/missing context raise errors. Standard Lua `load` remains unchanged.
`import_globals()` fails atomically if any selected name has a different value.

`collision` is a callable process-query namespace. Geometry stays explicit in
`mv.collision`. Graphs/animations supply automatic colliders; nil restores the
default and false disables it. Animations use a fixed union of local frame bounds.

## Migrate Lua handle operations

Malvado 2 uses a procedural Lua API inspired by DIV Games Studio. Managed
resource handles are passed to functions rather than manipulated through
instance methods.

The names on the right are available through `mv` or `import_globals()`. Use
the same handle for the new call. Object methods and the `mv.camera` and
`mv.world` aliases were removed.

| Previous call | Current call |
| --- | --- |
| `camera_create(x, y, {viewport = clip})` or `mv.camera.create(x, y, {viewport = clip})` | `camera_create {x = x, y = y, viewport = clip}` |
| `camera:set(x, y)` | `camera_set(camera, x, y)` |
| `camera:move(dx, dy)` | `camera_move(camera, dx, dy)` |
| `camera:follow(target)` | `camera_follow(camera, target)` |
| `camera:position()` | `camera_position(camera)` |
| `camera:destroy()` | `camera_destroy(camera)` |
| `mv.camera.set` | `mv.camera_set` |
| `mv.camera.move` | `mv.camera_move` |
| `mv.camera.follow` | `mv.camera_follow` |
| `mv.camera.position` | `mv.camera_position` |
| `region(x, y, width, height)` | `region {x = x, y = y, width = width, height = height}` |
| `mv.world.create()` | `world_create()` |
| `world:add_rect(options)` | `world_add_rect(world, options)` |
| `world:overlap(shape, transform)` | `world_overlap(world, shape, transform)` |
| `world:sweep(shape, transform, dx, dy)` | `world_sweep(world, shape, transform, dx, dy)` |
| `world:collision(player, position)` | `world_collision(world, player, position)` |
| `world:move(player, dx, dy)` or `mv.world.move(world, player, dx, dy)` | `world_move(world, player, dx, dy)` |
| `mv.world.move(dx, dy)` | `world_move(dx, dy)` using `self.world` |
| `self:recv()` | `recv(self)` |
| `definition:sample(elapsed_ms)` | `animation_sample(definition, elapsed_ms)` |
| `library:graph(id)` | `graph(library, id)` for atlases, FPG files, and manifests |
| `atlas:animation(name_or_ids, options)` | `animation(atlas, name_or_ids, options)` |
| `fpg:animation(ids, options)`, `manifest:animation(ids, options)` | `animation(library, ids, options)` with the same handle |
| `fpg:count()` | `fpg_count(fpg)` |
| `fpg:id_at(index)` | `fpg_id_at(fpg, index)` |
| `fpg:palette()` | `fpg_palette(fpg)` |
| `fpg:gamma()` | `fpg_gamma(fpg)` |
| `fpg:sprite(id)` | `fpg_sprite(fpg, id)` |
| `fpg:point(id, index)` | `fpg_point(fpg, id, index)` |
| `fpg:rgba(id, transparent_zero)` | `fpg_rgba(fpg, id, transparent_zero)` |
| `fpg:image(id, transparent_zero)` | `fpg_image(fpg, id, transparent_zero)` |
| `fpg:png(id, transparent_zero)` | `fpg_png(fpg, id, transparent_zero)` |
| `fpg:json(transparent_zero)` | `fpg_json(fpg, transparent_zero)` |
| `manifest:count()` | `manifest_count(manifest)` |
| `manifest:id_at(index)` | `manifest_id_at(manifest, index)` |
| `manifest:palette()` | `manifest_palette(manifest)` |
| `manifest:gamma()` | `manifest_gamma(manifest)` |
| `manifest:sprite(id)` | `manifest_sprite(manifest, id)` |
| `manifest:point(id, index)` | `manifest_point(manifest, id, index)` |
| `manifest:transparent_zero()` | `manifest_transparent_zero(manifest)` |

## Camera, region and scroll

`self.camera = nil` uses the main camera; false selects screen space; an explicit
camera selects its view. Camera movement leaves physical positions unchanged.
`region` clips in screen coordinates. `scroll` translates one process's drawing.
Their composition is world minus camera minus scroll plus viewport origin,
then intersection clipping. Use z to place a HUD above world processes.
Persistent text and fades retain their existing screen-space behavior.

Static `graph()` selection stays static. Automatic `animation()` uses the
simulation clock, independent of behavior fps, and pauses during sleep/freeze.
Repeated named selection does not restart. Custom ID lists should be created
once and reused. Explicit samplers remain available for manual control.

The [native animation example](../../examples/c/animation/) runs the same
playback math from a C process, with no Lua or graphics dependency.

## Native C and compatibility

Native asset parsers keep metadata and explicit ownership. The new
`malvado/animation.h` defines copied immutable timing sequences and caller-owned
player state. `malvado/camera.h` provides value-based projection, centering and
clipping. C games use these in their existing process callbacks and draw flow.
Lua context lookup, named caches and ergonomic aliases are binding work.
See [animations](../api/animations.md) and [cameras](../api/cameras.md) for C errors,
lifetimes, limits and SDK checks.

Modern selection uses `self.file`/`self.graph`. Historical FPG fields retain
meaning only in the explicit legacy profile. Malvado 1 compatibility is not an
alpha.6 requirement. Native `.aseprite` files, runtime ZIP mounting, extra
Aseprite metadata, complex camera composition and text/font redesign are not
introduced here. Immediate rectangle calls still take two corners; fill/color
and text signatures are unchanged.

## Reviewed runnable examples and evidence

| Example | Delivered tutorial style | Automated evidence |
| --- | --- | --- |
| [Collision](../../examples/collision/) | Global shapes and contextual process query | `lua_collision_example`, `visual_collision_example` |
| [World collision](../../examples/world-collision/) | Associated world and contextual movement | `lua_world_example`, `visual_world_example` |
| [Scroll](../../examples/scroll/) | Global region/scroll/shape constructors | `visual_scroll_example` |
| [Regions](../../examples/regions/) | Global region and graph constructor | `visual_regions_example` |
| [FPG](../../examples/fpg/) | Unified loader and contextual static graph IDs | `visual_fpg_example` |
| [Converted FPG](../../examples/fpg_converted/) | Same IDs/pivots from the manifest loader | `visual_fpg_conversion` |
| [Aseprite](../../examples/aseprite/) | Named automatic animation with native durations | `visual_aseprite_example`, `visual_aseprite_example_pixels` |
| [Sprites](../../examples/sprites/) | Manual static image selection at process-local fps | `visual_sprites_example`, `visual_cli_sprites_example` |
| [Demo](../../examples/demo/) | Minimal globals and unified PNG loading; advanced immediate sprite drawing | `visual_demo` |
| [Camera following](../../examples/camera-follow/) | Main camera and fixed HUD using start | Headless behavior and `visual_camera_follow_example` |
| [Camera menu](../../examples/camera-menu/) | Screen-space clipped local scrolling using game.init | Headless behavior and `visual_camera_menu_example` |
