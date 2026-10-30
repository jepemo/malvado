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

Run the [complete C and Lua examples](../../examples/c/processes/README.md), or try the [parity example](../../examples/c/parity/README.md) to compare their output. This page describes the current alpha.2 process API. For future graphics and other planned functions, see the [proposed Lua API](lua-contract.md).

`import_globals()` adds the modern Malvado functions to the Lua global environment. It checks all names before it adds any. If a name already has a different value, it raises an error and adds nothing. You can call it again when the installed functions are unchanged. Global names can still be shadowed by local variables; choose distinct local names. To avoid global names, use `local mv = require("malvado")` and call functions as `mv.process`, `mv.frame`, and `mv.start`. Both forms use the same API.

## Start and stop

`mv.process(fn)` returns a constructor. Call it with a table of initial fields, or with no argument, to start a process. The table's fields are copied into a new user-data object named `self`. The constructor returns a positive process ID. IDs increase and are not reused during the run.

`mv.start(init, options)` calls `init` once and runs until the window closes, a process fails, or no processes remain. `options` can set `width`, `height`, and `title`. The defaults are 800, 600, and `Malvado`. To run without a window, use `malvado-run --headless game.lua`. A headless game must end its processes to exit.

Call `mv.set_title(title)` after the window opens to change its title. It reports an error without a window or if the title contains a zero byte. In C, `mv_window_set_title(title)` returns 1 on success, or 0 for a null title or no window.

After `start` begins, `mv.window_size()` returns the current width and height in logical pixels. In headless mode it returns the configured size. In C, `mv_window_size(&width, &height)` returns 1 for an open window. Without a window it returns 0 and writes zeros; a null pointer returns 0 without writing.

`mv.key(name)` returns whether a key is held. Use lowercase letters `"a"` to `"z"`, digits `"0"` to `"9"`, `"left"`, `"right"`, `"up"`, `"down"`, `"space"`, `"escape"`, `"enter"`, `"tab"`, or `"backspace"`. Names are case-sensitive. Other names raise an error. In headless mode all supported keys return false. Try `"space"` and `"escape"` in the [C and Lua timing game](../../examples/c/timing-game/README.md).

The state is updated when the window processes events after a frame. It is a held state, not a text input or a press event. In C, use `mv_key_down` with an `MvKey`, such as `MV_KEY_LEFT`. Use `mv_key_from_name` to convert the same names to C key values. Both functions use the window thread.

Use `local x, y = mv.mouse_position()` for the pointer position in window pixels. The top-left corner is `(0, 0)`. Use `mv.mouse_button("left")`, `"middle"`, or `"right"` to check whether a button is held. An unknown name raises an error. In headless mode the position is `(0, 0)` and all buttons are released.

In C, `mv_mouse_position(&x, &y)` writes the position and returns 1 when a window is open. Without a window it writes zeros and returns 0. If either pointer is `NULL`, it returns 0 without writing. `mv_mouse_down(MV_MOUSE_LEFT)` checks a held button. `mv_mouse_button_from_name` converts the same names to `MvMouseButton` values. These functions use the window thread.

For a controller, use `mv.pad_connected(slot)`, `mv.pad_button(slot, name)`, and `mv.pad_axis(slot, name)`. Slots are 0 to 3 and can change when a controller disconnects. Button names are `a`, `b`, `x`, `y`, `up`, `down`, `left`, `right`, `lb`, `rb`, `back`, and `start`. Axis names are `left_x`, `left_y`, `right_x`, `right_y`, `left_trigger`, and `right_trigger`. Stick axes report values from -1 to 1 without a dead zone. Triggers report 0 when released and 1 when fully pressed, using the backend's standard mapping. Invalid slots or names raise an error. A disconnected controller or headless mode gives false for buttons and zero for axes. Events update after each frame. Actual mappings depend on the operating system and controller; physical-device testing remains pending.

In C, use `mv_pad_connected`, `mv_pad_button_down`, and `mv_pad_axis` with the `MvPadButton` and `MvPadAxis` enums. The name conversion functions return 0 for invalid input without changing the output. Invalid slots or controls, a disconnected controller, and no window return zero. Use these functions on the window thread.

For typed text, call `mv.text_input_next()` until it returns `nil`. Each call removes one character from the shared queue and returns it as a UTF-8 string. This is separate from `mv.key`: it follows the operating system's text input and can return characters with more than one byte. In headless mode it always returns `nil`.

In C, `mv_text_input_next(&codepoint)` removes one Unicode scalar and returns 1, or returns 0 without changing the value when there is no character. `mv_utf8_encode(codepoint, bytes)` writes up to four UTF-8 bytes and returns their count. The output is not NUL-terminated. Both APIs use the window thread.

The queue is shared by all processes and stays available across frames until read. It holds up to 256 characters and drops new characters when full. The window backend also has a per-frame event limit, so very large bursts can lose input. Closing the window clears the queue. Control keys such as Backspace are checked separately with `mv.key`.

`mv.frame()` yields until the next activation. By default this is the next tick. Set `self.fps` to an integer from 1 to 60 to limit that process to the requested number of activations per 60 simulation ticks; set it to 0 for every tick. The first activation is immediate when the scheduler starts the process. Fractional tick intervals alternate as needed, without cumulative drift. Changes take effect after the current activation. While a process is sleeping or frozen, its cadence does not advance. Its last drawing remains visible between activations. `frame` is valid only inside a running process. A process created during a tick starts no earlier than the next tick. An uncaught process error prints a traceback and gives the runner a nonzero exit code.

Use `mv.time_seconds()` to read a monotonic clock, including in headless mode. Its origin is unspecified. Subtract two readings to measure elapsed wall time; do not use it as a calendar time. It raises an error if the clock is unavailable. In C, `mv_monotonic_time(&seconds)` returns 1 on success or 0 without changing the output on failure.

Use `mv.rand(min, max)` for an integer in the inclusive range. Both limits must be integers and `min` must not exceed `max`. It uses Lua's `math.random` generator; use `math.randomseed` when you need a repeatable sequence. This is not a shared C/Lua random stream.

For persistent text, `mv.write(font, x, y, text, align)` returns an ID. Alignment is `"left"` (the default), `"center"`, or `"right"`; `x` is the matching horizontal anchor. Use `mv.change_text(id, text, x, y, font)` to change the text, position, or font. The last three arguments are optional; pass `nil` to keep a value. Use `mv.delete_text(id)` to remove it. Positions are window pixels from the top-left corner. `mv.font(size, r, g, b, a)` creates a font description using the built-in font; color channels default to 255. In C, `mv_window_text_width(text, size)` returns the built-in font width in pixels, or zero for invalid input or without a window.

To use a TTF or OTF file with persistent text, call `mv.load_font(path, size, r, g, b, a, extra_glyphs)` after the window opens. The size is an integer from 1 to 256 and the optional color channels default to 255. The optional last argument is a UTF-8 string of additional characters to load. For example, `mv.load_font("font.ttf", 24, nil, nil, nil, nil, "ΩЖ")` adds those two characters. Invalid UTF-8 and zero bytes are rejected. Pass the returned font to `mv.write` or `mv.change_text`. Keep the font description unchanged; the text keeps its font resource alive until the text is removed or the runner closes. Loading fails without a window or if the font cannot be loaded.

The loader always prepares printable ASCII and Latin-1 code points, plus any requested characters. It accepts up to 1024 distinct code points in total. A glyph also needs to exist in the font file. The built-in font has its own smaller glyph set. `mv.draw_text` uses the built-in font.

In C, open a window and call `mv_font_load(path, base_size)` for the default glyph set. To choose the whole set explicitly, use `mv_font_load_codepoints(path, base_size, codepoints, count)`, with 1 to 1024 distinct nonzero Unicode scalar values. Include a fallback character such as `?` in the set. Use `mv_font_text_width` and `mv_font_draw` with a size from 1 to 1024. They return 0 for invalid arguments or without a window; an empty string can also have zero width. Destroy every font with `mv_font_destroy` before closing the window. `mv_utf8_decode` and `mv_utf8_encode` convert one Unicode scalar at a time.

## Process data

Use `self` for your game fields, such as `self.remaining`. These fields are local to the process.

The fields `x`, `y`, `z`, `angle`, `fps`, `width`, and `height` default to 0; `size` defaults to 1. They are writable. Only `z` and `fps` affect modern process drawing and scheduling automatically. The legacy `frame` also uses the graphic fields described in the [compatibility guide](lua-contract.md#legacy-shim).

The following fields belong to the engine and cannot be assigned:

- `self.id`: this process ID.
- `self.parent`: the creator's ID, or `nil`. It keeps the original ID even if the parent later ends.
- `self.priority`: the current scheduler priority.
- `self.delta`: logical seconds since this process's previous activation, or 0 on its first activation. It includes ticks spent sleeping or frozen.
- `self.status`: `"running"`, `"sleeping"`, `"frozen"`, or `nil` after termination.
- `self.recv`: the message-receive method.

After a parent ends, its live children become roots in the process tree. Their `self.parent` still identifies their original creator. A saved `self` value can outlive its process. Its `id` and `parent` remain readable, but a priority lookup or receive operation on a terminated process raises an error.

## Control processes

`mv.status(id)` returns the same state strings, or `nil` for an unknown or terminated process. `mv.find(constructor)` returns a new table of live IDs for that type, in creation order. It includes sleeping and frozen processes. A C process with the same type ID is also included.

Use `mv.set_priority(id, value)` and `mv.get_priority(id)` to change and read integer priorities. The default is 0. Higher priorities run first; ties use creation order. The scheduler fixes the order at the start of a tick. A change during a tick takes effect on the next tick. Invalid IDs raise an error.

`mv.signal(id, signal)` accepts `"sleep"`, `"freeze"`, `"wakeup"`, or `"kill"`. `mv.signal_tree(id, signal)` also signals the live descendants at the time of the call. New children do not inherit that signal. `mv.kill(id)` and `mv.kill_tree(id)` are shortcuts for the kill signal.

Sleeping and frozen processes remain alive but do not run. Sleep hides their last process drawing; freeze keeps it visible. Wakeup makes them runnable again. A killed process cannot be reactivated. A process that kills itself continues until it yields or returns; cleanup happens afterward. Invalid signals or IDs raise an error without changing the tree.

## Send and receive messages

`mv.send(id, value)` puts a value in the recipient's FIFO queue. `self:recv()` returns the next value, or `nil` if the queue is empty. Sending `nil` is not allowed. A queue holds up to 1,024 messages. Sending to an unknown process or a full queue raises an error. Pending messages are released when the process ends.

Booleans, integers, numbers, and binary strings can be exchanged with C processes. Strings are copied. Tables and other Lua objects can be sent only to Lua processes; they retain their identity within the VM. See the [C contract](c-contract.md) for message ownership and release rules.

## Inspect the scheduler

`mv.snapshot()` returns a copy with `tick` and a `processes` list. Each entry contains `id`, `parent`, `type`, `status`, `priority`, `messages`, `draw_commands`, and `visible`. It includes live C and Lua processes. It does not copy message payloads or call game getters. Editing the result does not change the scheduler.

For tool authors, `src/runner/runtime.h` has experimental runner-thread hooks for snapshots, signals, pause, and one-tick stepping. These hooks are not a stable embedding API. They do not provide IPC or a GUI.
