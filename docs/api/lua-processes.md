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

Run the [complete C and Lua examples](../../examples/c/processes/README.md), or try the [parity example](../../examples/c/parity/README.md) to compare their output. This page describes the current process API. See the [alpha migration guide](../development/alpha5-process-migration.md) if your project uses numeric process IDs.

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

Once startup begins, `window_size()` returns the current width and height as
two integers in logical pixels. It works in the initializer and in processes.
In headless mode it returns the configured size. Calling it before startup
raises an error.

```lua
local width, height = window_size()
set_title("Level 2")
```

In C, `mv_window_size(&width, &height)` returns 1 for an open window.
Without a window it returns 0 and writes zeros. A null pointer returns 0
without writing.

The [input inventory](input-inventory.md) maps the C and Lua APIs to examples and tests.

Use `mv.key(mv.KEY_LEFT)` for continuous movement. Use `key_pressed` or
`key_released` for actions on the current logical tick. `mv.input.key` and
`mv.key` are the same function. After `import_globals()`, use `key(KEY_LEFT)`.

Use `mouse_button` for held buttons and `mouse_pressed` or `mouse_released`
for transitions. `mouse_position()` returns the pointer position in window
pixels, with `(0, 0)` at the top left.

```lua
if mouse_pressed(MOUSE_LEFT) then
  local x, y = mouse_position()
  print(x, y)
end
```

Controllers use explicit slots from 0 to 3. Use `pad_connected(slot)` to check
a slot, `pad_button(slot, PAD_A)` for held state, and `pad_pressed` or
`pad_released` for transitions. Use `pad_axis` for sticks and triggers. Sticks
report values from -1 to 1. Triggers report values from 0 to 1.

```lua
if pad_pressed(0, PAD_A) then
  play_sound(jump_sound)
end
local horizontal = pad_axis(0, PAD_LEFT_X)
```

The `KEY_*`, `MOUSE_*`, and `PAD_*` constants are strings. They are available
on `mv`, in `mv.input`, and after `import_globals()`. The functions also
accept the corresponding case-sensitive string values. Invalid names and
slots raise an error. Headless mode returns false for held and transition
queries, zero for axes, and `(0, 0)` for the pointer position.

Held and transition queries are shared and non-consuming. When a controller
connects, its first sampled held state establishes a baseline. Disconnecting
clears input for that slot and gives false for buttons and zero for axes.
Read the [logical tick contract](keyboard.md) for capture timing and the
[input inventory](input-inventory.md) for the matching C operations and constants.

For typed text, call `mv.text_input_next()` until it returns `nil`. Each call removes one character from the shared queue and returns it as a UTF-8 string. This is separate from `mv.key`: it follows the operating system's text input and can return characters with more than one byte. In headless mode it always returns `nil`.

In C, `mv_text_input_next(&codepoint)` removes one Unicode scalar and returns 1, or returns 0 without changing the value when there is no character. `mv_utf8_encode(codepoint, bytes)` writes up to four UTF-8 bytes and returns their count. The output is not NUL-terminated. Read the queue on the window thread. UTF-8 conversion does not require a window. `mv_utf8_decode(bytes, length, &codepoint)` reads one scalar and returns the bytes consumed. It returns 0 for invalid or incomplete UTF-8 and leaves the output unchanged. Extra bytes after the first scalar are not examined.

The queue is shared by all processes and stays available across frames until read. It holds up to 256 characters and drops new characters when full. The window backend also has a per-frame event limit, so very large bursts can lose input. Closing the window clears the queue. Control keys such as Backspace are checked separately with `mv.key`.

## Process timing

The Lua runner uses a fixed simulation step of 1/60 second. With a window,
it can run up to five catch-up ticks per rendered frame. In headless mode it
advances logical ticks without waiting for wall time.

`frame()` yields until the process's next activation. By default this is the
next tick. Set `self.fps` to an integer from 1 to 60 to set this process's
activation cadence. Set it to 0 for every tick. This field controls only
when the process runs. The simulation step and render cadence are independent.

The first activation is immediate when the scheduler starts the process. At a
constant rate from 1 through 60, each 60 eligible ticks after the first
activation produce the requested number of activations. Fractional tick
intervals alternate as needed, without cumulative drift. Changes take effect
after the current activation. While a process is sleeping or frozen, its
cadence does not advance. Its last drawing remains visible between activations.
`frame()` is valid only inside a running process. A process created during a
tick starts no earlier than the next tick. An uncaught process error prints a
traceback and gives the runner a nonzero exit code.

`self.delta` is the logical simulation time in seconds since the process's
previous activation. It starts at zero and includes skipped ticks, including
ticks spent sleeping or frozen. Multiply it by a speed in pixels per second
to move at that speed when the process cadence changes.

```lua
local Player = process(function(self)
  self.fps = 30
  local speed = 200 -- Pixels per second.
  while true do
    if key(KEY_RIGHT) then
      self.x = self.x + speed * self.delta
    end
    frame()
  end
end)
```

Use `time_seconds()` for profiling or to measure elapsed wall time, including
in headless mode. Subtract two readings from this monotonic clock. Its origin
is unspecified. It raises an error if the clock is unavailable. In C,
`mv_monotonic_time(&seconds)` returns 1 on success or 0 without changing the
output on failure.

## Pseudorandom numbers

Use `rand(min, max)` for a pseudorandom integer in the inclusive range. It
raises an error for non-integer bounds or a minimum greater than the maximum.
It shares Lua's `math.random` generator, so calls to either function advance
the same sequence.

Lua 5.4.8 automatically picks an initial seed when it opens the math library.
Ordinary gameplay needs no call to `rand_seed`. The initial seed provides weak
randomness. Use this generator for gameplay and keep it out of cryptographic
uses.

Call `rand_seed()` to ask Lua to generate a fresh seed. Call `rand_seed(seed)`
with one Lua integer, including zero or a negative integer, for repeatable
tests or procedural generation. Both forms return no values. Invalid calls,
including explicit `nil` or extra arguments, raise an error before the
generator state changes.

`math.randomseed` remains available directly. For repeatable results, use the
same seed and keep calls to `rand` and `math.random` in the same order.

```lua
require("malvado").import_globals()

local roll = rand(1, 6) -- Works without a manual seed.

rand_seed() -- Ask Lua for a fresh seed.

rand_seed(12345)
local damage = rand(1, 6)

math.randomseed(12345)
assert(rand(1, 6) == damage)
```

## Text

For persistent text, `mv.write {font = ..., text = ..., x = ..., y = ...}` returns a numeric ID. Alignment is `"left"` (the default), `"center"`, or `"right"`; `x` is the matching horizontal anchor. Optional `color` and `size` style each text independently while sharing one font. Use `mv.change_text(id, options)` to change only the fields present in the options table: `text`, `x`, `y`, `align`, `color`, `size` or `font`. Use `mv.delete_text(id)` to remove it. Use `mv.text_size(font, text)` or `mv.text_size(id)` to measure text in pixels without drawing it. Positions are window pixels from the top-left corner. Positions must be integers that fit a C `int`. Invalid input raises an error without creating or changing text. `write`, `change_text`, and `draw_text` reject strings containing zero bytes rather than truncating them. Colors are `{red, green, blue, alpha}` tables with four integers in 0..255 and are copied when a style is assigned. `mv.font {size = n}` creates a font descriptor using the built-in font. In C, `mv_window_text_width(text, size)` returns the built-in font width in pixels, or zero for invalid input, without a window, or when the width cannot fit in a C `int`.

Text created inside a process is removed when that process ends. Text created outside a process belongs to the game and remains until it is deleted or the game closes. Changing a text object does not change its owner.

To use a TTF or OTF file with persistent text, call `mv.load_font {path = ..., size = ...}` after the window opens. The size is an integer from 1 to 1024. The optional `extra_glyphs` field is a UTF-8 string of additional characters to load. For example, `mv.load_font {path = "font.ttf", size = 24, extra_glyphs = "ΩЖ"}` adds those two characters. Invalid UTF-8 and zero bytes are rejected. Pass the returned font to `mv.write` or `mv.change_text`. The text keeps its font resource alive until the text is removed or the runner closes. Loading fails without a window or if the font cannot be loaded.

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

The instance fields override defaults. Each spawn copies the default table graph, including cycles and shared references. Functions and other scalar values are shared. Tables with metatables and threads are rejected. Collision shapes, graphic/region/scroll descriptors, images, cameras, collision worlds, and asset-library userdata are shared; other userdata are rejected. A shared world remains the same mutable world; only ordinary Lua default tables are deep copied. The nesting limit is 256. Instance arguments are applied directly and are not deep copied. Validation of `z` and `fps` uses the resulting fields before the process is created.

## Process data

Use `self` for your game fields, such as `self.remaining`. These fields are local to the process.

The fields `x`, `y`, `z`, `angle`, `fps`, `width`, and `height` default to 0; `size` defaults to 1. They are writable. `x`, `y`, `angle`, `size`, and `z` control automatic process graphics; `fps` controls cadence. The legacy `frame` also uses the graphic fields described in the [compatibility guide](#legacy-shim).

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

Call `mv.set_priority(priority)` to set the current process's integer priority.
Call `mv.set_priority(handle, priority)` to set the priority of a specific live
process, including during setup. Read it with `mv.get_priority(handle)` or
`self.priority`. The default is 0. Higher priorities run first. Ties use creation
order. The scheduler fixes the order at the start of each tick, so a priority
change during a tick affects the next tick's order.

Call `mv.kill()` to stop the current process immediately. It ends with reason
`"killed"`. Execution does not continue after the call, even inside `pcall` or
`xpcall`. The scheduler runs `<close>` callbacks and releases the process's
resources and pending messages during cleanup.

Call `mv.kill(handle)` to target a specific process, including during setup.
It returns `true` when it kills a live target and returns `false` if the target
is already dead. If the call kills the calling process, it stops immediately,
as with `mv.kill()`.

The implicit forms require the live scheduled process coroutine. A nested Lua
coroutine cannot use implicit process control. Explicit handles must belong to
the current runtime. Invalid arguments or extra arguments raise an error. The
same forms are available through `mv.process` and `import_globals()`.

```lua
require("malvado").import_globals()

local Enemy = process(function(self)
  set_priority(10)

  while true do
    local message = recv()
    if message and message.damage then
      self.health = self.health - message.damage
    end
    if self.health <= 0 then
      kill()
    end
    frame()
  end
end)

start(function()
  local enemy = Enemy {health = 100}
  send(enemy, {damage = 100})
end)
```

`mv.signal(handle, signal)` accepts `"sleep"`, `"freeze"`, `"wakeup"`, or `"kill"`. `mv.signal_tree(handle, signal)` signals the live creation lineage at the time of the call, including through dead creators. Pass `{relation = "ownership"}` as the third argument to use the current ownership tree. New children do not inherit that signal. `mv.kill(handle)` and `mv.kill_tree(handle, options?)` are shortcuts for the kill signal. Pass `{relation = "ownership"}` as the second argument to `kill_tree` to select the ownership tree. Use `{relation = "creation"}` or omit the options to select the creation lineage. `mv.signal(handle, "kill")` has the same return behavior as `mv.kill(handle)`. `mv.process.alive(handle)` checks whether a process is live.

Sleeping and frozen processes remain alive but do not run. Sleep hides their last process drawing; freeze keeps it visible. Wakeup makes them runnable again. A killed process cannot be reactivated. A process stops immediately if `kill`, `signal`, `kill_tree`, or `signal_tree` kills it. Cleanup runs after the scheduler regains control. A self-kill in a non-yieldable Lua context raises an error before it changes the process state. Invalid signals or handles raise an error without changing the tree. If a close callback fails, Malvado reports the error, continues cleanup and ends the run with a nonzero result.

The scheduler controls process coroutines. `coroutine.resume()` and `coroutine.close()` return `false` and an error message if given a process coroutine. They work as usual for coroutines created by game code.

## Send and receive messages

`mv.send(handle, value)` puts a value in the recipient's FIFO queue. In the
scheduled process coroutine, use `mv.recv()` to remove and return the next
message from its own queue. Neither setup code nor an ordinary Lua coroutine
has an implicit recipient.

```lua
local Worker = process(function()
  while true do
    local message = recv()
    if message ~= nil then print(message) end
    frame()
  end
end)
```

Use `recv(handle)` to read from a specific live process, including during game
setup or from another process. Both forms return one `nil` for an empty queue
and return immediately. Pass zero arguments or one live process handle from the
same runtime. Explicit `nil`, numeric IDs, and extra arguments are errors.

Sending `nil` is an error. A queue holds up to 1,024 messages. Sending to an
unknown process or a full queue raises an error. Pending messages are released
when the process ends.

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

After importing globals, set `self.region` to `region {x = 40, y = 60, width = 300, height = 240}` to
clip this process's graph and immediate drawing. Set it to `nil` to remove the
clip. It does not change process coordinates or collision queries. See
[window clipping and regions](drawing.md#clip-a-lua-process-to-a-region).

## Process scrolling

After importing globals, assign `scroll(x, y)` to `self.scroll` to subtract
an absolute offset from this process's graph and immediate drawing. Set
`self.scroll = nil` to reset the offset. See [scrolling](drawing.md#scroll-a-lua-process).


## Process world context

Set `self.world` to a world from `world_create()` and call `world_move(dx, dy)`
inside the process to move with its existing collider. `collision(Enemy)` queries
that process against a process constructor or handle. Both names are installed
by `import_globals()`. Explicit calls remain available; see the
[collision and world contract](collision.md#contextual-process-and-world-calls).


## Process graphics library

`self.file` is an optional library from `load_asset()`. `graph(id)` selects from
that library inside the process. Assign the result to `self.graph` for automatic
static drawing. An explicit `graph(library, id)` uses the supplied library. Changing the file field cannot change an existing descriptor.
Images can also be assigned directly. See [static graphics](images.md#select-a-static-graphic)
for lifetime, pivot and context rules. Assign an `animation()` definition to
`self.graph` for [automatic playback](animations.md) with independent process
state and a fixed collider union.


## Process camera

`self.camera = nil` uses the main camera; `false` keeps graph/immediate drawing
in screen space. A camera from `camera_create {}` selects an explicit view.
Camera movement never changes process position or colliders. Use z to order
HUD content above world content. See [cameras](cameras.md) for projection,
viewport, validation and lifetime rules.

## Legacy shim

`require("malvado.compat.legacy").import_globals()` installs `process`, `frame`, `font`, `write`, `change_text`, `delete_text`, `key`, `clear_screen`, `set_title`, `get_screen_width`, `get_screen_height`, `load_image`, `time_seconds`, `rand`, `fpg`, `set_color`, `draw_rect`, `draw_circle`, `draw_line`, `fade_off`, and `fade_on` as globals. Use it only for migration. Repeating the call is safe. If any of those global names already holds a different value, the call raises an error and installs nothing. The normal namespace import does not inject globals. The constructor returns a process handle. Numeric process IDs are not accepted by process operations.

The legacy `frame()` draws the current process's `graph` or `fpg` before it yields. For `graph`, use an image returned by `load_image`. For a spritesheet, use `fpg(path, "image", rows, columns)` and set `self.fpgIndex` to a zero-based frame index. A negative or missing index hides the sheet; larger indices wrap. The image dimensions must divide evenly by the row and column counts. `self.x`, `self.y`, `self.angle`, and `self.size` control position, clockwise degrees, and positive scale. Their defaults are 0, 0, 0, and 1. Individual images use their center as pivot; sheet regions use their top-left corner. `self.z` and `self.fps` use the current draw order and cadence rules. After a legacy frame, `self.width` and `self.height` contain the scaled dimensions for individual and directory images. For a spritesheet or hidden library, both are 0, as in the historical implementation.

On macOS and Linux, `fpg(directory)` or `fpg(directory, "directory")` loads up to 4096 PNG files from the directory. It ignores other file extensions and sorts names by byte value, so the index is repeatable. Directory images use their center as pivot. An empty directory, invalid image, or unavailable directory raises an error. Directory mode is not yet supported on Windows.

This is a subset of the historical API. ZIP `fpg` mode, the general historical `fade` function, collision, and the `love` namespace are not implemented by the shim. The current examples use the modern API. Use the [image guide](images.md) and [drawing guide](drawing.md) for new code.
