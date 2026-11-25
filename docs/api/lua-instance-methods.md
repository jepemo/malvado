# Lua instance methods

Use procedural calls from the Malvado module or import them as globals. A
managed handle also supports optional method syntax for the operations below.
For example, both lines move the same camera by the same amount.

```lua
local mv = require("malvado")
local camera = mv.camera_create {}

mv.camera_move(camera, 10, 20)
camera:move(10, 20)
```

Lua passes the receiver as the first argument. You can also save the function
and pass the receiver yourself, as in `local move = camera.move` followed by
`move(camera, 10, 20)`. Both forms use the same operation, arguments, return
values, and resource rules as the explicit procedural call. Methods work
with or without `import_globals()`.

In the tables, `?` marks an optional argument. The procedural names are module
fields unless a namespace is shown.

## Cameras, worlds, images and animations

Camera methods select an explicit camera. World methods that act on a process
require its handle. Use the procedural forms for the main camera or the
current process and its world. An image requires an open window to load, and
animation descriptors require asset support.

| Instance method | Explicit procedural call |
| --- | --- |
| `camera:set(x, y)` | `camera_set(camera, x, y)` |
| `camera:move(dx, dy)` | `camera_move(camera, dx, dy)` |
| `camera:follow(target)` | `camera_follow(camera, target)` |
| `camera:position()` | `camera_position(camera)` |
| `camera:destroy()` | `camera_destroy(camera)` |
| `world:add_rect(rect)` | `world_add_rect(world, rect)` |
| `world:overlap(shape, transform?)` | `world_overlap(world, shape, transform?)` |
| `world:sweep(shape, options)` | `world_sweep(world, shape, options)` |
| `world:collision(process, options?)` | `world_collision(world, process, options?)` |
| `world:move(process, dx, dy)` | `world_move(world, process, dx, dy)` |
| `image:size()` | `image_size(image)` |
| `animation:sample(elapsed_ms)` | `animation_sample(animation, elapsed_ms)` |

See [cameras](cameras.md), [static worlds](collision.md), [images](images.md),
and [animation sampling](animations.md#explicit-sampling).

## Process handles

A built-in property such as `status` or `priority` still holds a value. A custom
field takes precedence over a method with the same name. This includes a field
set to `false` and a function you assign for use with `:`. Assign `nil` to a custom
field to restore method lookup.

After a process ends, its custom fields are released. The handle still lets you
call its methods. Each operation handles an ended process as its procedural
form does. For example, `handle:alive()` and a repeated `handle:kill()` return
`false`. A process that kills itself stops immediately. Sending a message with
`handle:send(message)` queues it for that handle. The `animation_elapsed` and
`animation_finished` methods require asset support.

| Instance method | Explicit procedural call |
| --- | --- |
| `handle:kill()` | `kill(handle)` |
| `handle:alive()` | `alive(handle)` |
| `handle:signal(signal)` | `signal(handle, signal)` |
| `handle:signal_tree(signal, options?)` | `signal_tree(handle, signal, options?)` |
| `handle:kill_tree(options?)` | `kill_tree(handle, options?)` |
| `handle:send(message)` | `send(handle, message)` |
| `handle:recv()` | `recv(handle)` |
| `handle:children(options?)` | `children(handle, options?)` |
| `handle:descendants(options?)` | `descendants(handle, options?)` |
| `handle:animation_elapsed()` | `animation_elapsed(handle)` |
| `handle:animation_finished()` | `animation_finished(handle)` |
| `handle:collision(target)` | `mv.process.collision(handle, target)` |
| `handle:collisions(target)` | `mv.process.collisions(handle, target)` |
| `handle:set_priority(priority)` | `set_priority(handle, priority)` |

See the [process guide](lua-processes.md) for the scheduler, mailbox, and tree
relations.

## Sound, voices and music

`sound:play(options)` starts one independent voice and returns its handle.
It returns `nil` if all voice slots are in use. Stop, pause, resume, and set the
volume through the voice handle. A music handle controls its stream. Loading
sound or music requires audio support and a graphical session.

| Instance method | Explicit procedural call |
| --- | --- |
| `sound:play(options?)` | `play_sound(sound, options?)` |
| `voice:stop()` | `stop_sound(voice)` |
| `voice:pause()` | `pause_sound(voice)` |
| `voice:resume()` | `resume_sound(voice)` |
| `voice:is_playing()` | `sound_playing(voice)` |
| `voice:set_volume(volume)` | `sound_volume(voice, volume)` |
| `music:play()` | `play_music(music)` |
| `music:update()` | `update_music(music)` |
| `music:stop()` | `stop_music(music)` |
| `music:pause()` | `pause_music(music)` |
| `music:resume()` | `resume_music(music)` |
| `music:is_playing()` | `music_playing(music)` |
| `music:set_volume(volume)` | `music_volume(music, volume)` |

See the [audio guide](audio.md) for playback and voice lifetime.

## Atlas, FPG and FPG manifest

These methods require asset support. The explicit graph selectors use the
library you pass and share the procedural graphic cache. A graph can supply
geometry in headless mode. Drawing it requires source images, and `fpg:image()`
requires an open window when it finds the requested sprite.

| Instance method | Explicit procedural call |
| --- | --- |
| `atlas:sprite_info(id_or_name)` | `atlas_sprite_info(atlas, id_or_name)` |
| `atlas:sprite_id(name)` | `atlas_sprite_id(atlas, name)` |
| `atlas:animation_frame(name, elapsed_ms, options?)` | `atlas_animation_frame(atlas, name, elapsed_ms, options?)` |
| `atlas:animation_color(name)` | `atlas_animation_color(atlas, name)` |
| `atlas:animation_repeats(name)` | `atlas_animation_repeats(atlas, name)` |
| `atlas:pivot_color()` | `atlas_pivot_color(atlas)` |
| `atlas:graph(id_or_name)` | `graph(atlas, id_or_name)` |
| `fpg:count()` | `fpg_count(fpg)` |
| `fpg:id_at(index)` | `fpg_id_at(fpg, index)` |
| `fpg:palette()` | `fpg_palette(fpg)` |
| `fpg:gamma()` | `fpg_gamma(fpg)` |
| `fpg:sprite(id)` | `fpg_sprite(fpg, id)` |
| `fpg:point(id, index)` | `fpg_point(fpg, id, index)` |
| `fpg:rgba(id, options?)` | `fpg_rgba(fpg, id, options?)` |
| `fpg:image(id, options?)` | `fpg_image(fpg, id, options?)` |
| `fpg:png(id, options?)` | `fpg_png(fpg, id, options?)` |
| `fpg:json(options?)` | `fpg_json(fpg, options?)` |
| `fpg:graph(id)` | `graph(fpg, id)` |
| `manifest:count()` | `manifest_count(manifest)` |
| `manifest:id_at(index)` | `manifest_id_at(manifest, index)` |
| `manifest:palette()` | `manifest_palette(manifest)` |
| `manifest:gamma()` | `manifest_gamma(manifest)` |
| `manifest:sprite(id)` | `manifest_sprite(manifest, id)` |
| `manifest:point(id, index)` | `manifest_point(manifest, id, index)` |
| `manifest:transparent_zero()` | `manifest_transparent_zero(manifest)` |
| `manifest:graph(id)` | `graph(manifest, id)` |

See [atlases](managed-atlases.md) and [FPG files](fpg.md) for format and loading
requirements.

## Other operations

Use `mv.collision.overlap(a, b, options)` to test geometric shapes. That form
keeps the two operands and their named `a` and `b` transforms together. Use
`get_priority(handle)` and `status(handle)` for procedural process reads, or
read `handle.priority` and `handle.status` as properties.

Use procedural calls for global services, contextual shortcuts such as
`world_move(dx, dy)` and `graph(selector)`, and constructors such as `animation`
and `parse_atlas`. Rectangle options and font descriptors are tables.
Persistent text operations select text by numeric ID. Regions, scrolls,
and graphics are values you pass to the drawing or process API.
