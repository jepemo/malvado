# Lua processes

Processes let your game run several tasks together. Each task keeps its Lua state when it calls `frame()`. Here is a small process that runs for three ticks:

```lua
require("malvado").import_globals()

local Counter = process(function(self)
  while self.remaining > 0 do
    print(self.remaining)
    self.remaining = self.remaining - 1
    if self.remaining > 0 then frame() end
  end
end)

start(function()
  Counter({remaining = 3})
end)
```

Run the [complete C and Lua examples](../../examples/c/processes/README.md), or try the [parity example](../../examples/c/parity/README.md) to compare their output. This page describes the current process API. See the [alpha migration guide](../development/alpha5-process-migration.md) if your project uses numeric process IDs. For future graphics and other planned functions, see the [proposed Lua API](lua-contract.md).

`import_globals()` adds the modern Malvado functions to the Lua global environment. It checks all names before it adds any. If a name already has a different value, it raises an error and adds nothing. If an allocation fails during installation, it removes the names added by that call. You can call it again when the installed functions are unchanged. Global names can still be shadowed by local variables; choose distinct local names. To avoid global names, use `local mv = require("malvado")` and call functions as `mv.process`, `mv.frame`, and `mv.start`. Both forms use the same API.

See the [global export table](lua-globals.md) for all current names.

## Start and stop

`mv.process` is a callable namespace. Use `mv.process(fn, options)` or `mv.process.define(fn, options)` to define a process. Both use the same implementation. Definition options are `defaults` and `children_attached`. Use `mv.process.spawn(definition, fields, options)` for an explicit spawn; its option is `attached`. Unknown options raise an error. Options use the table's own fields; metatable lookup is not used. Its current operations include `frame`, `signal`, `kill`, `find`, and `snapshot`; they use the same functions as the flat API.

`mv.process(fn)` returns a constructor. Call it with a table of initial fields, or with no argument, to start a process. The table's fields are copied into a new user-data object named `self`. The constructor returns the same userdata that the process receives as `self`. Its `id` is a positive process ID. IDs increase and are not reused during the run. Pass this userdata to process operations. Numeric IDs are for inspection and are not accepted as process handles. `find` returns canonical process handles.

`mv.start(init, options)` calls `init` once and runs until the window closes, a process fails, or no processes remain. `options` can set `width`, `height`, and `title`. The defaults are 800, 600, and `Malvado`. To run without a window, use `malvado run --headless game.lua`. A headless game must end its processes to exit.

You can also return a game table from the entry script. Set `width`, `height`, and `title` in the table. Add an `init` function to start processes. Malvado calls it once, without arguments, before the first tick. The same defaults apply. See the [declarative game example](../../examples/declarative/README.md).

The table can contain only those options and `init`. Dimensions must be positive integers that fit in a C `int`. The title must be a string without zero bytes. The table cannot have a metatable. A game cannot use both a returned table and `mv.start()` in the same run. If there is no initializer and no active process, the game exits.


Omit `options` or pass a table. Other values, including the historical debug boolean, raise an error. Invalid options do not start the game; correct them before trying again.

Call `mv.set_title(title)` after the window opens to change its title. It reports an error without a window or if the title contains a zero byte. In C, `mv_window_set_title(title)` returns 1 on success, or 0 for a null title or no window.

After `start` begins, `mv.window_size()` returns the current width and height in logical pixels. In headless mode it returns the configured size. In C, `mv_window_size(&width, &height)` returns 1 for an open window. Without a window it returns 0 and writes zeros; a null pointer returns 0 without writing.

The [input inventory](input-inventory.md) maps the C and Lua APIs to examples and tests.

`mv.key(name)` returns whether a key is held. Use lowercase letters `"a"` to `"z"`, digits `"0"` to `"9"`, `"left"`, `"right"`, `"up"`, `"down"`, `"space"`, `"escape"`, `"enter"`, `"tab"`, or `"backspace"`. Names are case-sensitive. Other names raise an error. In headless mode all supported keys return false. Try `"space"` and `"escape"` in the [C and Lua timing game](../../examples/c/timing-game/README.md).

The input namespace also provides `mv.input.key_down` and the other input functions. After `import_globals()`, `key_down` is the same function. Key names are also available as constants, including in `mv.input`. For example, use `mv.key(mv.KEY_UP)` or `mv.key(mv.KEY_A)`. After `import_globals()`, use `key(KEY_UP)`. Constants are strings, so `mv.KEY_UP` and `"up"` have the same value. Constants cover letters, digits, and the named keys above. Signal constants are `SIGNAL_SLEEP`, `SIGNAL_FREEZE`, `SIGNAL_WAKEUP`, and `SIGNAL_KILL`. They also have string values.

The state is updated when the window processes events after a frame. It is a held state, not a text input or a press event. In C, use `mv_key_down` with an `MvKey`, such as `MV_KEY_LEFT`. Use `mv_key_from_name` to convert the same names to C key values. It returns 0 without changing the output for an invalid name or null argument. Read key state on the window thread. Name conversion does not require a window.

Use `local x, y = mv.mouse_position()` for the pointer position in window pixels. The top-left corner is `(0, 0)`. Use `mv.mouse_button("left")`, `"middle"`, or `"right"` to check whether a button is held. An unknown name raises an error. In headless mode the position is `(0, 0)` and all buttons are released.

In C, `mv_mouse_position(&x, &y)` writes the position and returns 1 when a window is open. Without a window it writes zeros and returns 0. If either pointer is `NULL`, it returns 0 without writing. `mv_mouse_down(MV_MOUSE_LEFT)` checks a held button. `mv_mouse_button_from_name` converts the same names to `MvMouseButton` values. It returns 0 without changing the output for an invalid name or null argument. Read mouse state on the window thread. Name conversion does not require a window.

For a controller, use `mv.pad_connected(slot)`, `mv.pad_button(slot, name)`, and `mv.pad_axis(slot, name)`. Slots are 0 to 3 and can change when a controller disconnects. Button names are `a`, `b`, `x`, `y`, `up`, `down`, `left`, `right`, `lb`, `rb`, `back`, and `start`. Axis names are `left_x`, `left_y`, `right_x`, `right_y`, `left_trigger`, and `right_trigger`. Stick axes report values from -1 to 1 without a dead zone. Triggers report 0 when released and 1 when fully pressed, using the backend's standard mapping. Invalid slots or names raise an error. A disconnected controller or headless mode gives false for buttons and zero for axes. Events update after each frame. Actual mappings depend on the operating system and controller; physical-device testing remains pending.

In C, use `mv_pad_connected`, `mv_pad_button_down`, and `mv_pad_axis` with the `MvPadButton` and `MvPadAxis` enums. The name conversion functions return 0 for invalid input without changing the output. Invalid slots or controls, a disconnected controller, and no window return zero. Read controller state on the window thread. Name conversion does not require a window.

For typed text, call `mv.text_input_next()` until it returns `nil`. Each call removes one character from the shared queue and returns it as a UTF-8 string. This is separate from `mv.key`: it follows the operating system's text input and can return characters with more than one byte. In headless mode it always returns `nil`.

In C, `mv_text_input_next(&codepoint)` removes one Unicode scalar and returns 1, or returns 0 without changing the value when there is no character. `mv_utf8_encode(codepoint, bytes)` writes up to four UTF-8 bytes and returns their count. The output is not NUL-terminated. Read the queue on the window thread. UTF-8 conversion does not require a window. `mv_utf8_decode(bytes, length, &codepoint)` reads one scalar and returns the bytes consumed. It returns 0 for invalid or incomplete UTF-8 and leaves the output unchanged. Extra bytes after the first scalar are not examined.

The queue is shared by all processes and stays available across frames until read. It holds up to 256 characters and drops new characters when full. The window backend also has a per-frame event limit, so very large bursts can lose input. Closing the window clears the queue. Control keys such as Backspace are checked separately with `mv.key`.

`mv.frame()` yields until the next activation. By default this is the next tick. Set `self.fps` to an integer from 1 to 60 to limit that process to the requested number of activations per 60 simulation ticks; set it to 0 for every tick. The first activation is immediate when the scheduler starts the process. At a constant rate from 1 through 60, each 60 eligible ticks after the first activation produce the requested number of activations. Fractional tick intervals alternate as needed, without cumulative drift. Changes take effect after the current activation. While a process is sleeping or frozen, its cadence does not advance. Its last drawing remains visible between activations. `frame` is valid only inside a running process. A process created during a tick starts no earlier than the next tick. An uncaught process error prints a traceback and gives the runner a nonzero exit code.

Use `mv.time_seconds()` to read a monotonic clock, including in headless mode. Its origin is unspecified. Subtract two readings to measure elapsed wall time; do not use it as a calendar time. It raises an error if the clock is unavailable. In C, `mv_monotonic_time(&seconds)` returns 1 on success or 0 without changing the output on failure.

Use `mv.rand(min, max)` for an integer in the inclusive range. Both limits must be integers and `min` must not exceed `max`. It uses Lua's `math.random` generator; use `math.randomseed` when you need a repeatable sequence. This is not a shared C/Lua random stream.

```lua
math.randomseed(12345)
local damage = mv.rand(1, 6) -- Both 1 and 6 are possible.
```

`rand` is also available after `import_globals()`. It raises an error for a
non-integer bound or a minimum greater than the maximum. `lua_rand` checks the
full integer range, fixed bounds, invalid arguments, and seeded parity with
Lua's generator.

For persistent text, `mv.write(font, x, y, text, align)` returns an ID. Alignment is `"left"` (the default), `"center"`, or `"right"`; `x` is the matching horizontal anchor. Use `mv.change_text(id, text, x, y, font)` to change the text, position, or font. The last three arguments are optional; pass `nil` to keep a value. Use `mv.delete_text(id)` to remove it. Positions are window pixels from the top-left corner. Positions must be integers that fit a C `int`. Invalid positions raise an error without creating or changing text. `write`, `change_text`, and `draw_text` reject strings containing zero bytes rather than truncating them. `mv.font(size, r, g, b, a)` creates a font description using the built-in font; color channels default to 255. In C, `mv_window_text_width(text, size)` returns the built-in font width in pixels, or zero for invalid input, without a window, or when the width cannot fit in a C `int`.

Text created inside a process is removed when that process ends. Text created outside a process belongs to the game and remains until it is deleted or the game closes. Changing a text object does not change its owner.

To use a TTF or OTF file with persistent text, call `mv.load_font(path, size, r, g, b, a, extra_glyphs)` after the window opens. The size is an integer from 1 to 256 and the optional color channels default to 255. The optional last argument is a UTF-8 string of additional characters to load. For example, `mv.load_font("font.ttf", 24, nil, nil, nil, nil, "ΩЖ")` adds those two characters. Invalid UTF-8 and zero bytes are rejected. Pass the returned font to `mv.write` or `mv.change_text`. Keep the font description unchanged; the text keeps its font resource alive until the text is removed or the runner closes. Loading fails without a window or if the font cannot be loaded.

The loader always prepares printable ASCII and Latin-1 code points, plus any requested characters. It accepts up to 1024 distinct code points in total. A glyph also needs to exist in the font file. The built-in font has its own smaller glyph set. `mv.draw_text` uses the built-in font. Call it inside a running process with text, integer `x` and `y`, and an optional integer size (default 20). Coordinates and size must fit a C `int`; out-of-range values raise an error without adding a draw command.

In C, open a window and call `mv_font_load(path, base_size)` for the default glyph set. To choose the whole set explicitly, use `mv_font_load_codepoints(path, base_size, codepoints, count)`, with 1 to 1024 distinct nonzero Unicode scalar values. Include a fallback character such as `?` in the set. Use `mv_font_text_width` and `mv_font_draw` with a size from 1 to 1024. They return 0 for invalid arguments or without a window; an empty string can also have zero width. Destroy every font with `mv_font_destroy` before closing the window. `mv_utf8_decode` and `mv_utf8_encode` convert one Unicode scalar at a time.

## Find related processes

`mv.process.children(parent)` returns the live direct children of an owner. `mv.process.descendants(parent)` includes all live descendants. Both return a new table of canonical handles in creation order. Sleeping and frozen processes are included.

Pass `{relation = "creation"}` to follow creator relationships, including through dead ancestors. Pass `{type = Enemy}` to filter by a process definition. For example, `mv.process.descendants(parent, {relation = "creation", type = Enemy})` returns matching descendants. Ownership is the default relation. Unknown query or signal-tree options raise an error before changing processes. These queries use the current scheduler relations.

## Children and ownership

A process started outside an active process is a root. This includes creation before `start` or inside its initial callback. An explicit `{attached = true}` requires an active owner and raises an error without one. An explicit `{attached = false}` creates a root. No process can be created during full runtime shutdown.

A process started inside another process is attached to its owner by default. It ends when its owner ends. To create an independent child, use `mv.process.spawn(Child, {x = 10}, {attached = false})`. Its `creator_id` still identifies the creator, but its `parent` is `nil`.

`children_attached` is a reserved Boolean field. Set `children_attached = false` in a process definition or its creation fields to make its future children independent by default. A process can change `self.children_attached` for future children. The explicit `attached` spawn option has priority. During deferred cleanup, a close callback must use `spawn` with `attached = false` to create a process. Runtime shutdown rejects new processes.

## Default fields

Pass `defaults` in the second argument to `mv.process` to set fields for each instance:

```lua
local Enemy = mv.process(function(self)
  print(self.health, self.stats.attack)
end, {defaults = {health = 100, stats = {attack = 10}}})

Enemy {health = 250}
```

The instance fields override defaults. Each spawn copies the default table graph, including cycles and shared references. Functions and other scalar values are shared. Tables with metatables and threads are rejected. Collision shapes, graphic descriptors, and image userdata are shared; other userdata are rejected. The nesting limit is 256. Instance arguments are applied directly and are not deep copied. Validation of `z` and `fps` uses the resulting fields before the process is created.

## Process data

Use `self` for your game fields, such as `self.remaining`. These fields are local to the process.

The fields `x`, `y`, `z`, `angle`, `fps`, `width`, and `height` default to 0; `size` defaults to 1. They are writable. `x`, `y`, `angle`, `size`, and `z` control automatic process graphics; `fps` controls cadence. The legacy `frame` also uses the graphic fields described in the [compatibility guide](lua-contract.md#legacy-shim).

A failed spawn leaves no process or scheduler entry from that attempt. It does not change the supplied fields or defaults. Lua prepares the process state before the native spawn. If preparation or native creation fails, it releases the references from that attempt.

The following fields belong to the engine and cannot be assigned. Do not put them in defaults or creation fields; Malvado rejects them before creating the process:

- `self.id`: this process ID.
- `self.creator_id`: the creator’s ID, or `nil` for a root.
- `self.parent`: the current owner handle, or `nil`.
- `self.priority`: the current scheduler priority.
- `self.delta`: logical seconds since this process's previous activation, or 0 on its first activation. It includes ticks spent sleeping or frozen.
- `self.status`: `"running"`, `"sleeping"`, `"frozen"`, or `"dead"` after termination.
- `self.reason`: `nil` while alive, then `"returned"`, `"killed"`, or `"error"`.
- `self.recv`: the message-receive method.

After an owner ends, its surviving children become roots in the current process tree. Their `self.creator_id` keeps the original creator ID. A saved `self` value can outlive its process. Its `id`, `status`, and `reason` remain readable. Other field reads and writes raise an error after termination. Cleanup releases its game fields, even while Lua still holds `self`.

The same rules apply to handles for native C processes. During Lua shutdown, a finalizer can still read the identity, dead status, and reason of a retained handle. Other fields are unavailable. Handles belong to one runtime and Lua state; they cannot be used in a later run.

## Control processes

`mv.status(handle)` returns the current state string, or `nil` for a terminated process. `mv.find(constructor)` returns a new table of live handles for that type, in creation order. It includes sleeping and frozen processes. A C process with the same type ID is also included.

Use `mv.set_priority(handle, value)` and `mv.get_priority(handle)` to change and read integer priorities. The default is 0. Higher priorities run first; ties use creation order. The scheduler fixes the order at the start of a tick. A change during a tick takes effect on the next tick. Invalid handles raise an error.

`mv.signal(handle, signal)` accepts `"sleep"`, `"freeze"`, `"wakeup"`, or `"kill"`. `mv.signal_tree(handle, signal)` signals the live creation lineage at the time of the call, including through dead creators. Pass `{relation = "ownership"}` as the third argument to use the current ownership tree. New children do not inherit that signal. `mv.kill(handle)` and `mv.kill_tree(handle)` are shortcuts for the kill signal. `mv.kill(handle)` and `mv.signal(handle, "kill")` return true when they kill a live process and false if that handle is already dead. `mv.process.alive(handle)` checks whether a process is live.

Sleeping and frozen processes remain alive but do not run. Sleep hides their last process drawing; freeze keeps it visible. Wakeup makes them runnable again. A killed process cannot be reactivated. A process stops immediately if `kill`, `signal`, `kill_tree`, or `signal_tree` kills it. Cleanup runs after the scheduler regains control. A self-kill in a non-yieldable Lua context raises an error before it changes the process state. Invalid signals or handles raise an error without changing the tree. If a close callback fails, Malvado reports the error, continues cleanup and ends the run with a nonzero result.

The scheduler controls process coroutines. `coroutine.resume()` and `coroutine.close()` return `false` and an error message if given a process coroutine. They work as usual for coroutines created by game code.

## Send and receive messages

`mv.send(handle, value)` puts a value in the recipient's FIFO queue. `self:recv()` returns the next value, or `nil` if the queue is empty. Sending `nil` is not allowed. A queue holds up to 1,024 messages. Sending to an unknown process or a full queue raises an error. Pending messages are released when the process ends.

Booleans, integers, numbers, and binary strings can be exchanged with C processes. Strings are copied. Tables and other Lua objects can be sent only to Lua processes; they retain their identity within the VM. See the [C contract](c-contract.md) for message ownership and release rules.

## Inspect the scheduler

`mv.snapshot()` returns a copy with a 48-character hexadecimal `runtime` identity, `tick`, and a `processes` list. The runtime identity stays the same during one run and is different with overwhelming probability after a restart. Each entry contains `id`, `parent`, `type`, `status`, `priority`, `messages`, `draw_commands`, and `visible`. It includes live C and Lua processes. It does not copy message payloads or call game getters. Editing the result does not change the scheduler.

For tool authors, `src/runner/runtime.h` has experimental runner-thread hooks for snapshots, signals, pause, and one-tick stepping. These hooks are not a stable embedding API. They do not provide IPC or a GUI.


## Process graphics and collision

Assign a reusable `graphics.circle` or `graphics.rect` descriptor to `self.graph`
to draw it at the process position. When `self.collider` is `nil`, the graphic
also provides a collision shape. Set an explicit collider to override it, or
set `false` to disable collision. See [drawing](drawing.md#process-graphics)
and [collision](collision.md#process-queries-in-lua) for transforms, lifetime,
and error behavior. Images and atlas sprite descriptors can also provide automatic colliders. Use `process.collision(self, Enemy)` for the first matching process or `process.collisions(self, Enemy)` for all matches in creation order.

## Process drawing region

After importing globals, set `self.region` to `region(x, y, width, height)` to
clip this process's graph and immediate drawing. Set it to `nil` to remove the
clip. It does not change process coordinates or collision queries. See
[window clipping and regions](drawing.md#clip-a-lua-process-to-a-region).

## Process scrolling

After importing globals, assign `scroll(x, y)` to `self.scroll` to draw this
process relative to a world offset. The explicit constructor is
`mv.graphics.scroll(x, y)`. Other processes and collision positions
are unchanged. See [scrolling](drawing.md#scroll-a-lua-process).


## Process world context

Set `self.world` to a world from `world_create()` and call `world_move(dx, dy)`
inside the process to move with its existing collider. `collision(Enemy)` queries
that process against a process constructor or handle. Both names are installed
by `import_globals()`. Explicit calls remain available; see the
[collision and world contract](collision.md#contextual-process-and-world-calls).
