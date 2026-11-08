# Lua namespace function inventory

This list covers functions in the `mv` namespaces. It complements the [global
export table](lua-globals.md), which records flat functions and constants. The
table links to contracts and examples; it does not certify every semantic test
or select the complete 2.0 API.

`mv.process` is also callable to define a process. Process and input constants
are listed in the global table. Functions marked with * require asset support.
The `world` and FPG objects have methods documented in their linked guides;
object methods and process fields are not included in this namespace table.

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
| `mv.process.set_priority` | Change priority | [processes](lua-processes.md) and [collision](collision.md) |
| `mv.process.get_priority` | Read priority | [processes](lua-processes.md) and [collision](collision.md) |
| `mv.process.kill` | Kill a process | [processes](lua-processes.md) and [collision](collision.md) |
| `mv.process.alive` | Check a handle | [processes](lua-processes.md) and [collision](collision.md) |
| `mv.process.kill_tree` | Kill a process tree | [processes](lua-processes.md) and [collision](collision.md) |
| `mv.process.status` | Read process status | [processes](lua-processes.md) and [collision](collision.md) |
| `mv.process.send` | Send a message | [processes](lua-processes.md) and [collision](collision.md) |
| `mv.process.collision` | Find the first colliding process | [processes](lua-processes.md) and [collision](collision.md) |
| `mv.process.collisions` | List colliding processes | [processes](lua-processes.md) and [collision](collision.md) |
| `mv.input.key_down` | Check a held key | [input](input-inventory.md) |
| `mv.input.mouse_position` | Read pointer position | [input](input-inventory.md) |
| `mv.input.mouse_button` | Check a held mouse button | [input](input-inventory.md) |
| `mv.input.pad_connected` | Check a controller slot | [input](input-inventory.md) |
| `mv.input.pad_button` | Check a held controller button | [input](input-inventory.md) |
| `mv.input.pad_axis` | Read a controller axis | [input](input-inventory.md) |
| `mv.input.text_input_next` | Read one typed character | [input](input-inventory.md) |
| `mv.graphics.circle` | Create a circle graphic | [drawing](drawing.md) and [images](images.md) |
| `mv.graphics.rect` | Create a rectangle graphic | [drawing](drawing.md) and [images](images.md) |
| `mv.graphics.region` | Create a drawing region | [drawing](drawing.md) and [images](images.md) |
| `mv.graphics.scroll` | Create a scroll descriptor | [drawing](drawing.md) and [images](images.md) |
| `mv.graphics.sprite` | Create an atlas sprite graphic * | [drawing](drawing.md) and [images](images.md) |
| `mv.collision.circle` | Create a circle shape | [collision](collision.md) |
| `mv.collision.rect` | Create a rectangle shape | [collision](collision.md) |
| `mv.collision.overlap` | Test two transformed shapes | [collision](collision.md) |
| `mv.world.create` | Create a static collision world | [collision](collision.md) |
| `mv.fpg.parse` | Parse an FPG file * | [FPG](../specs/fpg-8.md) |
| `mv.fpg.parse_manifest` | Parse an FPG manifest * | [FPG](../specs/fpg-8.md) |

The `lua_namespace_inventory` test compares this table with functions exposed
by the runtime. It checks both asset-enabled and asset-disabled builds when
those builds run; it does not verify function behavior.
