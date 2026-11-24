# Lua global exports

`require("malvado").import_globals()` installs the names below. It checks all names before it changes globals. A different existing value causes an error. If installation fails, it removes the names added by that call. Repeating the import with the same values is safe. Normal `require` does not install globals.

See the [namespace inventory](lua-namespaces.md) for functions that are also available through `mv` namespaces.
Use one options table with `camera_create(options)` and `region(options)`.
Use two positional offsets with `scroll(x, y)`. See [cameras](cameras.md)
and [process drawing](drawing.md#clip-a-lua-process-to-a-region).

Call `recv()` and `animation_elapsed()` inside the scheduled process coroutine.
Pass a live process handle to either function to select a specific process.
See [messages](lua-processes.md#send-and-receive-messages) and
[animations](animations.md#explicit-sampling).

Call `kill()` or `set_priority(priority)` to control the live scheduled process
from its own coroutine. Use `kill(handle)` or `set_priority(handle, priority)`
to target a process by handle. See [process control](lua-processes.md#control-processes).

Each global has the same value as the `mv` field shown below. `process` and `collision` are callable tables. Calling `collision(target)` queries the current process; `mv.collision` keeps its geometric constructors and tests. `mv.process.define` defines a process and is not a separate global. `mv.process.collision` and `mv.process.collisions` are namespace-only functions; they are not separate globals. Most process operations and the signal constants are also available in `mv.process`. Use `recv` or `mv.recv` to receive messages. Input operations and constants are also available in `mv.input`. The `graph`/`animation` selectors, `animation_sample`, atlas functions, and `fpg_*` and `manifest_*` operations require asset support. The legacy module has a separate import; it is not included in this table.

| Global | Module field |
| --- | --- |
| `camera_follow` | `mv.camera_follow` |
| `camera_create` | `mv.camera_create` |
| `camera_set` | `mv.camera_set` |
| `camera_move` | `mv.camera_move` |
| `camera_position` | `mv.camera_position` |
| `camera_destroy` | `mv.camera_destroy` |
| `collision` | `mv.collision` |
| `rect` | `mv.rect` |
| `circle` | `mv.circle` |
| `region` | `mv.region` |
| `scroll` | `mv.scroll` |
| `world_create` | `mv.world_create` |
| `world_add_rect` | `mv.world_add_rect` |
| `world_overlap` | `mv.world_overlap` |
| `world_sweep` | `mv.world_sweep` |
| `world_collision` | `mv.world_collision` |
| `world_move` | `mv.world_move` |
| `rect_overlaps` | `mv.rect_overlaps` |
| `rect_contains` | `mv.rect_contains` |
| `process` | `mv.process` |
| `spawn` | `mv.spawn` |
| `find` | `mv.find` |
| `children` | `mv.children` |
| `descendants` | `mv.descendants` |
| `snapshot` | `mv.snapshot` |
| `frame` | `mv.frame` |
| `signal` | `mv.signal` |
| `signal_tree` | `mv.signal_tree` |
| `set_priority` | `mv.set_priority` |
| `get_priority` | `mv.get_priority` |
| `kill` | `mv.kill` |
| `alive` | `mv.alive` |
| `kill_tree` | `mv.kill_tree` |
| `status` | `mv.status` |
| `send` | `mv.send` |
| `recv` | `mv.recv` |
| `start` | `mv.start` |
| `set_fps` | `mv.set_fps` |
| `set_title` | `mv.set_title` |
| `set_window_size` | `mv.set_window_size` |
| `window_size` | `mv.window_size` |
| `set_color` | `mv.set_color` |
| `fade_to` | `mv.fade_to` |
| `fade_active` | `mv.fade_active` |
| `fade_color` | `mv.fade_color` |
| `time_seconds` | `mv.time_seconds` |
| `clear_screen` | `mv.clear_screen` |
| `key` | `mv.key` |
| `key_pressed` | `mv.key_pressed` |
| `key_released` | `mv.key_released` |
| `mouse_position` | `mv.mouse_position` |
| `mouse_button` | `mv.mouse_button` |
| `mouse_pressed` | `mv.mouse_pressed` |
| `mouse_released` | `mv.mouse_released` |
| `pad_connected` | `mv.pad_connected` |
| `pad_button` | `mv.pad_button` |
| `pad_pressed` | `mv.pad_pressed` |
| `pad_released` | `mv.pad_released` |
| `pad_axis` | `mv.pad_axis` |
| `text_input_next` | `mv.text_input_next` |
| `text_input_set_keyboard` | `mv.text_input_set_keyboard` |
| `load_sound` | `mv.load_sound` |
| `play_sound` | `mv.play_sound` |
| `stop_sound` | `mv.stop_sound` |
| `pause_sound` | `mv.pause_sound` |
| `resume_sound` | `mv.resume_sound` |
| `sound_playing` | `mv.sound_playing` |
| `sound_volume` | `mv.sound_volume` |
| `load_music` | `mv.load_music` |
| `music_playing` | `mv.music_playing` |
| `music_volume` | `mv.music_volume` |
| `draw_text` | `mv.draw_text` |
| `animation` | `mv.animation` |
| `graph` | `mv.graph` |
| `load_asset` | `mv.load_asset` |
| `load_image` | `mv.load_image` |
| `image_size` | `mv.image_size` |
| `draw_image` | `mv.draw_image` |
| `load_atlas` | `mv.load_atlas` |
| `load_aseprite` | `mv.load_aseprite` |
| `sprite_info` | `mv.sprite_info` |
| `sprite_id` | `mv.sprite_id` |
| `animation_frame` | `mv.animation_frame` |
| `animation_sample` | `mv.animation_sample` |
| `animation_elapsed` | `mv.animation_elapsed` |
| `animation_color` | `mv.animation_color` |
| `atlas_pivot_color` | `mv.atlas_pivot_color` |
| `animation_repeats` | `mv.animation_repeats` |
| `draw_sprite` | `mv.draw_sprite` |
| `fpg_count` | `mv.fpg_count` |
| `fpg_id_at` | `mv.fpg_id_at` |
| `fpg_palette` | `mv.fpg_palette` |
| `fpg_gamma` | `mv.fpg_gamma` |
| `fpg_sprite` | `mv.fpg_sprite` |
| `fpg_point` | `mv.fpg_point` |
| `fpg_rgba` | `mv.fpg_rgba` |
| `fpg_image` | `mv.fpg_image` |
| `fpg_png` | `mv.fpg_png` |
| `fpg_json` | `mv.fpg_json` |
| `manifest_count` | `mv.manifest_count` |
| `manifest_id_at` | `mv.manifest_id_at` |
| `manifest_palette` | `mv.manifest_palette` |
| `manifest_gamma` | `mv.manifest_gamma` |
| `manifest_sprite` | `mv.manifest_sprite` |
| `manifest_point` | `mv.manifest_point` |
| `manifest_transparent_zero` | `mv.manifest_transparent_zero` |
| `font` | `mv.font` |
| `load_font` | `mv.load_font` |
| `write` | `mv.write` |
| `change_text` | `mv.change_text` |
| `delete_text` | `mv.delete_text` |
| `text_size` | `mv.text_size` |
| `fade_off` | `mv.fade_off` |
| `fade_on` | `mv.fade_on` |
| `draw_rect` | `mv.draw_rect` |
| `draw_circle` | `mv.draw_circle` |
| `draw_line` | `mv.draw_line` |
| `rand` | `mv.rand` |
| `rand_seed` | `mv.rand_seed` |
| `play_music` | `mv.play_music` |
| `update_music` | `mv.update_music` |
| `stop_music` | `mv.stop_music` |
| `pause_music` | `mv.pause_music` |
| `resume_music` | `mv.resume_music` |
| `KEY_A` | `mv.KEY_A` |
| `KEY_B` | `mv.KEY_B` |
| `KEY_C` | `mv.KEY_C` |
| `KEY_D` | `mv.KEY_D` |
| `KEY_E` | `mv.KEY_E` |
| `KEY_F` | `mv.KEY_F` |
| `KEY_G` | `mv.KEY_G` |
| `KEY_H` | `mv.KEY_H` |
| `KEY_I` | `mv.KEY_I` |
| `KEY_J` | `mv.KEY_J` |
| `KEY_K` | `mv.KEY_K` |
| `KEY_L` | `mv.KEY_L` |
| `KEY_M` | `mv.KEY_M` |
| `KEY_N` | `mv.KEY_N` |
| `KEY_O` | `mv.KEY_O` |
| `KEY_P` | `mv.KEY_P` |
| `KEY_Q` | `mv.KEY_Q` |
| `KEY_R` | `mv.KEY_R` |
| `KEY_S` | `mv.KEY_S` |
| `KEY_T` | `mv.KEY_T` |
| `KEY_U` | `mv.KEY_U` |
| `KEY_V` | `mv.KEY_V` |
| `KEY_W` | `mv.KEY_W` |
| `KEY_X` | `mv.KEY_X` |
| `KEY_Y` | `mv.KEY_Y` |
| `KEY_Z` | `mv.KEY_Z` |
| `KEY_0` | `mv.KEY_0` |
| `KEY_1` | `mv.KEY_1` |
| `KEY_2` | `mv.KEY_2` |
| `KEY_3` | `mv.KEY_3` |
| `KEY_4` | `mv.KEY_4` |
| `KEY_5` | `mv.KEY_5` |
| `KEY_6` | `mv.KEY_6` |
| `KEY_7` | `mv.KEY_7` |
| `KEY_8` | `mv.KEY_8` |
| `KEY_9` | `mv.KEY_9` |
| `KEY_ESCAPE` | `mv.KEY_ESCAPE` |
| `KEY_SPACE` | `mv.KEY_SPACE` |
| `KEY_LEFT` | `mv.KEY_LEFT` |
| `KEY_RIGHT` | `mv.KEY_RIGHT` |
| `KEY_UP` | `mv.KEY_UP` |
| `KEY_DOWN` | `mv.KEY_DOWN` |
| `KEY_ENTER` | `mv.KEY_ENTER` |
| `KEY_TAB` | `mv.KEY_TAB` |
| `KEY_BACKSPACE` | `mv.KEY_BACKSPACE` |
| `MOUSE_LEFT` | `mv.MOUSE_LEFT` |
| `MOUSE_MIDDLE` | `mv.MOUSE_MIDDLE` |
| `MOUSE_RIGHT` | `mv.MOUSE_RIGHT` |
| `PAD_A` | `mv.PAD_A` |
| `PAD_B` | `mv.PAD_B` |
| `PAD_X` | `mv.PAD_X` |
| `PAD_Y` | `mv.PAD_Y` |
| `PAD_UP` | `mv.PAD_UP` |
| `PAD_DOWN` | `mv.PAD_DOWN` |
| `PAD_LEFT` | `mv.PAD_LEFT` |
| `PAD_RIGHT` | `mv.PAD_RIGHT` |
| `PAD_LB` | `mv.PAD_LB` |
| `PAD_RB` | `mv.PAD_RB` |
| `PAD_BACK` | `mv.PAD_BACK` |
| `PAD_START` | `mv.PAD_START` |
| `PAD_LEFT_X` | `mv.PAD_LEFT_X` |
| `PAD_LEFT_Y` | `mv.PAD_LEFT_Y` |
| `PAD_RIGHT_X` | `mv.PAD_RIGHT_X` |
| `PAD_RIGHT_Y` | `mv.PAD_RIGHT_Y` |
| `PAD_LEFT_TRIGGER` | `mv.PAD_LEFT_TRIGGER` |
| `PAD_RIGHT_TRIGGER` | `mv.PAD_RIGHT_TRIGGER` |
| `SIGNAL_SLEEP` | `mv.SIGNAL_SLEEP` |
| `SIGNAL_FREEZE` | `mv.SIGNAL_FREEZE` |
| `SIGNAL_WAKEUP` | `mv.SIGNAL_WAKEUP` |
| `SIGNAL_KILL` | `mv.SIGNAL_KILL` |

## Palette globals

See the [Malvado palette](drawing.md#malvado-palette) for channel values.

| Global | Module field |
| --- | --- |
| `BLACK` | `mv.colors.BLACK` |
| `WHITE` | `mv.colors.WHITE` |
| `TRANSPARENT` | `mv.colors.TRANSPARENT` |
| `GRAY` | `mv.colors.GRAY` |
| `LIGHT_GRAY` | `mv.colors.LIGHT_GRAY` |
| `DARK_GRAY` | `mv.colors.DARK_GRAY` |
| `RED` | `mv.colors.RED` |
| `GREEN` | `mv.colors.GREEN` |
| `BLUE` | `mv.colors.BLUE` |
| `YELLOW` | `mv.colors.YELLOW` |
| `CYAN` | `mv.colors.CYAN` |
| `MAGENTA` | `mv.colors.MAGENTA` |
| `ORANGE` | `mv.colors.ORANGE` |
| `PURPLE` | `mv.colors.PURPLE` |
| `PINK` | `mv.colors.PINK` |
| `BROWN` | `mv.colors.BROWN` |

## Module-only functions

These functions are available from `require("malvado")` and are not installed
by the normal global import:

- `mv.import_globals`: install the globals listed above, with conflict checks.
- `mv.legacy_fpg`: load a legacy image sheet or directory. The legacy import
  exposes it as `fpg`; see the [legacy guide](lua-processes.md#legacy-shim).

The other module tables are mapped in the [namespace inventory](lua-namespaces.md).
`lua_module_inventory` checks module fields against these two inventories; it
checks names and kinds, not behavior.
