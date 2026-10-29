# Initial Malvado 2 Lua contract

**Proposed design. Some parts are not yet implemented.** These are Malvado names, not official BennuGD signatures. Main API: `local mv = require("malvado")`. The normal import does not export globals. A legacy shim may enable them.

| Proposed signature | Initial contract | Implementation |
| --- | --- | --- |
| `mv.start(init, options?)` | Create a context/window and call init once; run until close, exit, or no game processes | C runner/binding |
| `mv.process(fn)` → constructor | Define a process type; `fn(self)` runs in a VM coroutine | C |
| `Constructor{...}` → pid | Create an instance and copy fields; run on the next tick | C |
| `mv.frame()` | Yield until the next tick; error outside a process | `lua_yield` from C |
| `mv.signal(pid, signal)` | Typed signal; an unknown handle is an error in the modern profile | C |
| `mv.kill(pid)` | Kill signal alias; no implicit tree in the modern profile | C |
| `mv.send(pid, value)` | Queue a value; error for an invalid target or a full queue | C/registry |
| `self:recv()` → value/nil | Dequeue without blocking; nil is not a payload | C/registry |
| `mv.set_proc(pid, values)` | Update validated fields; identity fields are read-only | C |
| `mv.key(name)` → bool | Held state for the tick; documented names adapted by the backend | C |
| `mv.set_title(text)` | Change the title; document reduced behavior on Android | C/raylib |
| `mv.clear_screen(r,g,b,a?)` | Set a persistent background; integer channels 0–255, alpha defaults to 255 | C |
| `mv.font(size,r?,g?,b?,a?)` → font | Context-owned font; color defaults to white | C |
| `mv.write(font,x,y,text,align?)` → text_id | Persistent text; alignment defaults to top left | C |
| `mv.change_text(id,text,x?,y?,font?)` | Change text; omitted arguments retain their values | C |
| `mv.delete_text(id)` | Explicit removal; define behavior for an already deleted ID in P0 | C |
| `mv.load_image(path)` → image | Asset relative to the project; load errors include the path | C |
| `mv.rand(min,max)` → integer | Inclusive range; engine RNG with pinned algorithm and seed | C |
| `mv.time_seconds()` → number | Monotonic time since startup; replaces `love.timer.getTime` in demos | C |
| `mv.exit(code?)` | Request orderly shutdown, default code 0; end the current process in a controlled way | C |

`options` specifies `width`, `height`, `title`, and `tick_rate`. Defaults are 800×600, Malvado, and 60. Unify profiles, packaging, and CLI configuration before implementation so that defaults have one source.

Proposed initial properties: read-only `id`, `x/y/z=0`, `angle=0` in degrees, `size=1`, `graph=nil`, and `fps=0` (each tick), plus user fields. Before P3, define `file/fpg/fpgIndex`, dimensions, anchor, and `delta` units. Do not assign Bennu semantics automatically. In the new profile, `delta` will be logical seconds between activations. Document the previous behavior and the difference in the legacy profile.

## Proposed basic example

```lua
local mv = require("malvado")
local Hello = mv.process(function(self)
  local font = mv.font(40, 255, 255, 255)
  local text = mv.write(font, self.x, self.y, "Hello, Malvado")
  while not mv.key("escape") do
    mv.frame()
  end
  mv.delete_text(text)
end)

mv.start(function()
  mv.clear_screen(25, 30, 45)
  Hello { x = 60, y = 80 }
end, { width = 800, height = 600, title = "My game" })
```

## Proposed legacy shim

`require("malvado.compat.legacy").import_globals()` installs `process`, `frame`, `font`, `write`, `key`, and other globals. Use it only for migration. The normal namespace import must not inject globals. The constructor still returns a PID, like the current core. The shim adapts names and arguments; runtime work remains in C. It does not emulate the full `love` namespace. Replace direct LÖVE calls in examples.
