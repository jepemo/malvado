# Move alpha process code to handles

Malvado 2.0 does not promise compatibility with 1.0 or earlier alpha APIs. Use this guide to update a project that used the numeric process API from an earlier 2.0 alpha.

A constructor now returns a process handle. Keep that handle and pass it to process functions:

```lua
local mv = require("malvado")
local Enemy = mv.process(function(self)
  mv.process.frame()
end)

local enemy = Enemy {}
mv.process.signal(enemy, mv.process.SIGNAL_FREEZE)
mv.process.kill(enemy)
```

Inside a process, pass `self` rather than `self.id`. The `id` field is for inspection; it is not a process handle. `find`, `children`, `descendants`, and `parent` return handles. A dead handle keeps `id`, `status`, and `reason`.

Children now depend on their owner by default. Use `mv.process.spawn(Enemy, {}, {attached = false})` for a child that must survive its creator. `creator_id` keeps the historical creator ID. `parent` gives the current owner handle, or `nil` for an independent process.

The names in `mv.process` and `mv.input` are the preferred grouped API. The matching flat `mv` names are 2.0 shortcuts to the same functions. `import_globals()` adds the documented global shortcuts only when requested. These shortcuts do not promise 1.0 behavior. The existing `malvado.compat.legacy` module is experimental and is not a 2.0 compatibility requirement.

C process IDs remain context-local identifiers. Use `MvProcessRef` when you must validate a reference against a context. The C API is new and can change before beta.
