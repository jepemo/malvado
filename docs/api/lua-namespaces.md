# Lua namespace function inventory

This list covers functions in the `mv` namespaces. It complements the [global
export table](lua-globals.md), which records flat functions and constants. The
table links to contracts and examples; it does not certify every semantic test
or select the complete 2.0 API.

`mv.process` is also callable to define a process. Process and input constants
are listed in the global table. Functions marked with * require asset support.
See the [palette](drawing.md#malvado-palette) for constants under `mv.colors`.
Pass managed handles to the procedural functions in the global export table.
Process fields are documented in the [process guide](lua-processes.md).
See [instance methods](lua-instance-methods.md) for optional handle syntax.

| Function | Purpose | Contract and examples |
| --- | --- | --- |
| `mv.process.define` | Define a process | [processes](lua-processes.md) and [collision](collision.md) |
| `mv.process.spawn` | Start a process | [processes](lua-processes.md) and [collision](collision.md) |
| `mv.process.find` | Find processes by type | [processes](lua-processes.md) and [collision](collision.md) |
| `mv.process.children` | List direct children | [processes](lua-processes.md) and [collision](collision.md) |
| `mv.process.descendants` | List descendants | [processes](lua-processes.md) and [collision](collision.md) |
| `mv.process.snapshot` | Inspect processes | [processes](lua-processes.md) and [collision](collision.md) |
| `mv.process.frame` | Yield a process | [processes](lua-processes.md) and [collision](collision.md) |
| `mv.process.signal` | Send a signal | [processes](lua-processes.md) and [collision](collision.md) |
| `mv.process.signal_tree` | Signal a process tree | [processes](lua-processes.md) and [collision](collision.md) |
| `mv.process.set_priority` | Set priority with `(priority)` or `(handle, priority)` | [processes](lua-processes.md#control-processes) |
| `mv.process.get_priority` | Read priority | [processes](lua-processes.md) and [collision](collision.md) |
| `mv.process.kill` | Kill the current process with `()` or select a process with `(handle)` | [processes](lua-processes.md#control-processes) |
| `mv.process.alive` | Check a handle | [processes](lua-processes.md) and [collision](collision.md) |
| `mv.process.kill_tree` | Kill a process tree | [processes](lua-processes.md) and [collision](collision.md) |
| `mv.process.status` | Read process status | [processes](lua-processes.md) and [collision](collision.md) |
| `mv.process.send` | Send a message | [processes](lua-processes.md) and [collision](collision.md) |
| `mv.process.collision` | Find the first colliding process | [processes](lua-processes.md) and [collision](collision.md) |
| `mv.process.collisions` | List colliding processes | [processes](lua-processes.md) and [collision](collision.md) |
| `mv.input.key` | Check a held key | [input](input-inventory.md) |
| `mv.input.key_pressed` | Check a press in the current tick | [keyboard](keyboard.md) |
| `mv.input.key_released` | Check a release in the current tick | [keyboard](keyboard.md) |
| `mv.input.mouse_position` | Read pointer position | [input](input-inventory.md) |
| `mv.input.mouse_button` | Check a held mouse button | [input](input-inventory.md) |
| `mv.input.mouse_pressed` | Check a mouse button press in the current tick | [input](input-inventory.md) |
| `mv.input.mouse_released` | Check a mouse button release in the current tick | [input](input-inventory.md) |
| `mv.input.pad_connected` | Check a controller slot | [input](input-inventory.md) |
| `mv.input.pad_button` | Check a held controller button | [input](input-inventory.md) |
| `mv.input.pad_pressed` | Check a controller button press in the current tick | [input](input-inventory.md) |
| `mv.input.pad_released` | Check a controller button release in the current tick | [input](input-inventory.md) |
| `mv.input.pad_axis` | Read a controller axis | [input](input-inventory.md) |
| `mv.input.text_input_next` | Read one typed character | [input](input-inventory.md) |
| `mv.input.text_input_set_keyboard` | Request the Android on-screen keyboard | [keyboard](keyboard.md#android-on-screen-keyboard) |
| `mv.graphics.circle` | Create a circle graphic | [drawing](drawing.md) and [images](images.md) |
| `mv.graphics.rect` | Create a rectangle graphic | [drawing](drawing.md) and [images](images.md) |
| `mv.graphics.region` | Create a drawing region | [drawing](drawing.md) and [images](images.md) |
| `mv.graphics.scroll` | Create a scroll descriptor | [drawing](drawing.md) and [images](images.md) |
| `mv.graphics.animation` | Select automatic animation * | [animations](animations.md) |
| `mv.graphics.animation_elapsed` | Inspect process playback time * | [animations](animations.md) |
| `mv.graphics.graph` | Select a static library graphic * | [images](images.md) |
| `mv.collision.circle` | Create a circle shape from a table with `radius` and optional offsets | [collision](collision.md) |
| `mv.collision.rect` | Create a rectangle shape from a table with `width`, `height`, and optional offsets | [collision](collision.md) |
| `mv.collision.overlap` | Test two shapes with optional transforms in `options.a` and `options.b` | [collision](collision.md) |
| `mv.fpg.parse` | Parse an FPG file * | [FPG](fpg.md) |
| `mv.fpg.parse_manifest` | Parse an FPG manifest * | [FPG](fpg.md) |

The `lua_namespace_inventory` test compares this table with functions exposed
by the runtime. It checks both asset-enabled and asset-disabled builds when
those builds run; it does not verify function behavior.
