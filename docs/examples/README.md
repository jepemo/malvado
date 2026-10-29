# Examples as an engine contract

The new snippets below are proposals, not programs that all run on the current runtime. The original examples in `examples/` require the historical LÖVE engine, available in [Git history](https://github.com/jepemo/malvado/tree/4670d8a). Keep the examples until their replacements are verified. Keep reference traces and screenshots.

## First-class C and Lua examples

Each shared engine capability will have a C example that uses the public API and a Lua example that uses the official binding. One program can demonstrate several functions. Documentation pages will link to the actual files, and CI will run them. See [API documentation](../development/api-documentation.md). Give priority to hello, processes/trees/signals, and messages. Add a mixed C/Lua game to verify the shared scheduler and context. The headless C example must build and run without Lua or raylib. The graphics version uses the raylib backend. Snippets and targets remain proposals until the applicable [C architecture](../architecture/c-first.md) is implemented.

## Migrate existing examples

| Current example | Behavior to retain | Required work |
| --- | --- | --- |
| `examples/helloworld/main.lua` | Process constructor, font, text, Escape, and automatic shutdown | Namespace/legacy shim, C fonts, and runner |
| `examples/demo/main.lua` | Menu, spritesheets, keyboard, text, and fades | Persistent resources, animation, z, and per-process cadence |
| `examples/space_asteroids/main.lua` | Complete game, movement, bullets, and collision | Remove `love.math.random`/`love.graphics.points`; map input and graphics |
| `examples/bench/main.lua` | Process creation, load, and timing | Replace `love.timer.getTime`; separate VM, scheduler, and render costs |

Compare benchmarks on the same machine with the same process count, resources, VSync, and target FPS. Measure CPU per tick, render time, memory, and process creation/removal cost. Fix the random seed and publish the results. Do not promise a speedup before measurement.

## Minimum new collection

| Example | Functions shown | Check |
| --- | --- | --- |
| `hello` | start/process/frame/font/write | Greeting is visible; Escape exits without leaks |
| `process-family` | PID, creation, parentage, kill-tree | Expected tree and activation trace |
| `signals` | sleep/freeze/wakeup/kill | Counters and visibility distinguish states |
| `messages` | send/recv | FIFO, documented shared reference, dead destination |
| `timing` | Fixed tick, per-process cadence | No accumulated drift at different render FPS |
| `sprites` | image, atlas, pivot, angle, size, z | Screenshot covers rotation, scale, mirror, and z ties |
| `collision` | AABB, then mask | Edge contact and different sizes |
| `text` | write/change/delete, alignment, UTF-8 | Score changes without recreating objects each tick |
| `input` | Keyboard, mouse, controller | Held key versus event and hotplug |
| `audio` | Effect, music, volume, and pause | Resume without duplicate channels |
| `scroll` | Region and camera | Large scene with fixed HUD |
| `fpg-import` | Import one FPG variant to PNG+JSON | Preserve non-consecutive IDs, points, transparency, and palette |
| `atlas-import` | Aseprite PNG+JSON atlas subset | Durations, offsets, and explicit rejection of unsupported options |
| `gui-workflow` | Open a copy of starter/asteroids from the GUI | Create, run, navigate to error, inspect ticks, and export with CLI services |
| `legacy-formats` | FPG/MAP/FNT/PAL when available | Asset with known controls and palette |
| `asteroids` | Complete game and export | Start/play/lose/restart cycle on each target |
| `android-touch` | Touch, Back, save, and lifecycle | Play after device lock/unlock |
| `native-process` | C callback with explicit state | Scheduler yields without suspending the C stack |

Each folder will have a README with controls and behavior, requirements, commands, licensed assets, a screenshot when useful, and expected output or trace when applicable. Lua examples have `main.lua`; C examples have `main.c` and build configuration. Add `malvado.toml` when the project format exists and applies. The README identifies profiles and minimum version. Include snippets from actual example files in the site. Do not maintain a separate manual copy.

## Proposed message example

```lua
local mv = require("malvado")
local Receiver = mv.process(function(self)
  while true do
    local msg = self:recv()
    if msg then
      print(msg.kind)
      return -- end the process so the runtime can close
    end
    mv.frame()
  end
end)

mv.start(function()
  local pid = Receiver {}
  mv.send(pid, { kind = "hello" })
end)
```

Acceptance: print `hello` once. The queue holds the message until the receiver starts on the next tick. Add checks for FIFO with multiple messages and for removal of a receiver with pending messages.

Add [GUI](../specs/gui.md) and [assets/FPG](../specs/assets.md) flows and fixtures when those capabilities exist. `gui-workflow` is a product acceptance test on the same games, not a new engine or a divergent copy of asteroids.

## Do not declare an example migrated only because it builds

Use headless tests for API contracts when possible. Test rendering and input in a real run. Screenshots can allow visual tolerances for fonts and GPUs. Android examples must be playable with touch and no external keyboard. CI must run examples without a window when they have a trace mode and run window smoke tests on available targets.
