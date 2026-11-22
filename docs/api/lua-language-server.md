# Lua Language Server support

Malvado ships LuaCATS type definitions for the complete public Lua API. The
[Lua Language Server](https://luals.github.io/) (LuaLS) reads them and provides
autocompletion, type checking and hover documentation in VS Code and other
editors.

The definitions describe the public API without executing runtime
functionality. They live in the SDK under `types/`:

| Path | Contents |
| --- | --- |
| `types/lib/*.d.lua` | The `malvado` module, its namespaces, constants and all shared option types. |
| `types/globals/malvado-globals.d.lua` | The global functions installed by `import_globals()`. |

## Generated projects

`malvado new PATH` writes a `.luarc.json` file into the project when the
installed SDK provides the definitions. Open the project folder in VS Code and
install the [Lua](https://marketplace.visualstudio.com/items?itemName=sumneko.lua)
extension. Completion, signature help and hover documentation work without
further configuration.

The generated `.luarc.json` points at the SDK `types/lib` and `types/globals`
directories that were current when the project was created. If you move or
reinstall the SDK, recreate the project or edit the paths. No absolute path is
hard-coded into the engine; the CLI resolves the installed SDK at generation
time, so the same flow works on Linux, macOS and Windows.

Projects that use only the namespaced API can remove the `types/globals`
entry to keep the global namespace clean:

```json
{
  "runtime.version": "Lua 5.4",
  "workspace.library": [
    "<SDK>/types/lib"
  ],
  "workspace.checkThirdParty": false
}
```

## Manual setup for existing projects

Copy the `.luarc.json` above into your project and replace `<SDK>` with your
Malvado SDK location. Keep the `types/globals` entry when the entry point
calls `require("malvado").import_globals()`.

No custom VS Code extension is required. Any editor that runs LuaLS reads the
same definitions.

## What you get

Hover any function for its purpose, parameters, defaults and return values:

```lua
local mv = require("malvado")

local f = mv.font {size = 50}

local title = mv.write {
  font = f,
  text = "Hello, Malvado!",
  x = 400,
  y = 300,
  align = "center",
  color = {255, 255, 255, 255},
}

mv.change_text(title, {
  color = {255, 100, 100, 180},
})
```

While typing `mv.write {`, LuaLS offers the option fields, rejects unknown
fields, and suggests the accepted `align` values. Colors check as
`{red, green, blue, alpha}` tables of four integers in 0..255.

## Declare custom process fields

Process bodies accept any custom field, and the definitions allow reading and
writing them without annotations:

```lua
local Enemy = process(function(self)
  self.health = 100
  self.x = 40
  while true do frame() end
end)
```

For completion and type checking on your own fields, declare a subclass of the
built-in process type and cast `self` at the start of the body:

```lua
---@class Enemy : MvProcess
---@field health integer

local Enemy = process(function(self)
  ---@cast self Enemy
  self.health = 100
  self.health = self.health - 10
  while true do frame() end
end)
```

`MvProcess` documents every built-in field: `x`, `y`, `z`, `angle`, `size`,
`fps`, `graph`, `camera`, `region`, `scroll`, `collider`,
`children_attached`, and the read-only `id`, `parent`, `creator_id`,
`priority`, `status`, `reason`, `delta` and `recv`.

## Coverage

The definitions cover processes and lifecycle, process fields, signals,
graphics and drawing, sprites and animations, text and fonts, input, audio
and music, resource loading, the namespaced API and the opt-in globals. The
implementation is the source of truth; definitions follow the delivered API.
