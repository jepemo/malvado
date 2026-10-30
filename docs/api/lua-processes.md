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

`mv.frame()` yields until the next tick. It is valid only inside a running process. A process created during a tick starts no earlier than the next tick. An uncaught process error prints a traceback and gives the runner a nonzero exit code.

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
