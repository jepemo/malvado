# Lua processes

Processes let your game run several tasks together. Each task keeps its Lua state when it calls `mv.frame()`. Here is a small process that runs for three ticks:

```lua
local mv = require("malvado")

local Counter = mv.process(function(self)
  while self.remaining > 0 do
    print(self.remaining)
    self.remaining = self.remaining - 1
    if self.remaining > 0 then mv.frame() end
  end
end)

mv.start(function()
  Counter({remaining = 3})
end)
```

Run the [complete C and Lua examples](../../examples/c/processes/README.md), or try the [parity example](../../examples/c/parity/README.md) to compare their output. This page describes the current alpha.2 process API. For future graphics and other planned functions, see the [proposed Lua API](lua-contract.md).

## Start and stop

`mv.process(fn)` returns a constructor. Call it with a table of initial fields, or with no argument, to start a process. The table's fields are copied into a new user-data object named `self`. The constructor returns a positive process ID. IDs increase and are not reused during the run.

`mv.start(init, options)` calls `init` once and runs until the window closes, a process fails, or no processes remain. `options` can set `width`, `height`, and `title`. The defaults are 800, 600, and `Malvado`. To run without a window, use `malvado-run --headless game.lua`. A headless game must end its processes to exit.

`mv.key(name)` returns whether a key is held. Use lowercase letters `"a"` to `"z"`, digits `"0"` to `"9"`, `"left"`, `"right"`, `"up"`, `"down"`, `"space"`, `"escape"`, `"enter"`, `"tab"`, or `"backspace"`. Names are case-sensitive. Other names raise an error. In headless mode all supported keys return false. Try `"space"` and `"escape"` in the [C and Lua timing game](../../examples/c/timing-game/README.md).

The state is updated when the window processes events after a frame. It is a held state, not a text input or a press event. In C, use `mv_key_down` with an `MvKey`, such as `MV_KEY_LEFT`. Use `mv_key_from_name` to convert the same names to C key values. Both functions use the window thread.

Use `local x, y = mv.mouse_position()` for the pointer position in window pixels. The top-left corner is `(0, 0)`. Use `mv.mouse_button("left")`, `"middle"`, or `"right"` to check whether a button is held. An unknown name raises an error. In headless mode the position is `(0, 0)` and all buttons are released.

In C, `mv_mouse_position(&x, &y)` writes the position and returns 1 when a window is open. Without a window it writes zeros and returns 0. If either pointer is `NULL`, it returns 0 without writing. `mv_mouse_down(MV_MOUSE_LEFT)` checks a held button. `mv_mouse_button_from_name` converts the same names to `MvMouseButton` values. These functions use the window thread.

For a controller, use `mv.pad_connected(slot)`, `mv.pad_button(slot, name)`, and `mv.pad_axis(slot, name)`. Slots are 0 to 3 and can change when a controller disconnects. Button names are `a`, `b`, `x`, `y`, `up`, `down`, `left`, `right`, `lb`, `rb`, `back`, and `start`. Axis names are `left_x`, `left_y`, `right_x`, and `right_y`. Stick axes report values from -1 to 1 without a dead zone. Invalid slots or names raise an error. A disconnected controller or headless mode gives false for buttons and zero for axes. Events update after each frame. Actual mappings depend on the operating system and controller; physical-device testing remains pending.

In C, use `mv_pad_connected`, `mv_pad_button_down`, and `mv_pad_axis` with the `MvPadButton` and `MvPadAxis` enums. The name conversion functions return 0 for invalid input without changing the output. Invalid slots or controls, a disconnected controller, and no window return zero. Use these functions on the window thread.

For typed text, call `mv.text_input_next()` until it returns `nil`. Each call removes one character from the shared queue and returns it as a UTF-8 string. This is separate from `mv.key`: it follows the operating system's text input and can return characters with more than one byte. In headless mode it always returns `nil`.

In C, `mv_text_input_next(&codepoint)` removes one Unicode scalar and returns 1, or returns 0 without changing the value when there is no character. `mv_utf8_encode(codepoint, bytes)` writes up to four UTF-8 bytes and returns their count. The output is not NUL-terminated. Both APIs use the window thread.

The queue is shared by all processes and stays available across frames until read. It holds up to 256 characters and drops new characters when full. The window backend also has a per-frame event limit, so very large bursts can lose input. Closing the window clears the queue. Control keys such as Backspace are checked separately with `mv.key`.

`mv.frame()` yields until the next tick. It is valid only inside a running process. A process created during a tick starts no earlier than the next tick. An uncaught process error prints a traceback and gives the runner a nonzero exit code.

Use `mv.time_seconds()` to read a monotonic clock, including in headless mode. Its origin is unspecified. Subtract two readings to measure elapsed wall time; do not use it as a calendar time. It raises an error if the clock is unavailable. In C, `mv_monotonic_time(&seconds)` returns 1 on success or 0 without changing the output on failure.

For persistent text, `mv.write(font, x, y, text)` returns an ID. Use `mv.change_text(id, text, x, y, font)` to change it. The last three arguments are optional; pass `nil` to keep a value. Use `mv.delete_text(id)` to remove it. Positions are window pixels from the top-left corner. `mv.font(size, r, g, b, a)` creates a font description using the built-in font; color channels default to 255.

## Process data

Use `self` for your game fields, such as `self.remaining`. These fields are local to the process. The following fields belong to the engine and cannot be assigned:

- `self.id`: this process ID.
- `self.parent`: the creator's ID, or `nil`. It keeps the original ID even if the parent later ends.
- `self.priority`: the current scheduler priority.
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
