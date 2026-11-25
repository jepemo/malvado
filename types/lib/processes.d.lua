-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0
--
-- Process types for the Lua Language Server (LuaLS).

---@meta

---Signal name accepted by `signal` and `signal_tree`.
---@alias MvSignal "sleep"|"freeze"|"wakeup"|"kill"

---Status string for a live process.
---@alias MvLiveProcessStatusName "running"|"sleeping"|"frozen"

---Status string for a live or retained process handle.
---@alias MvProcessStatusName MvLiveProcessStatusName|"dead"

---A live process handle. The same value is `self` inside a process body.
---
---Built-in fields are listed here. Add your own fields freely:
---
---```lua
---local Enemy = process(function(self)
---  self.health = 100        -- any custom field
---  self.x = 40
---  while true do frame() end
---end)
---```
---
---To document custom fields for the editor, declare a subclass and cast:
---
---```lua
----@class Enemy : MvProcess
----@field health integer
---
---local Enemy = process(function(self)
---  ---@cast self Enemy
---  self.health = 100
---  ...
---end)
---```
---@class MvProcess
---@field id integer Read-only unique process ID.
---@field x number Horizontal position.
---@field y number Vertical position.
---@field z number Draw order. Higher values draw later.
---@field angle number Rotation in degrees.
---@field size number Scale factor. Default 1.
---@field fps integer Activations per 60 eligible logical ticks (0..60). Both 0 and 60 run every tick. The initial value comes from instance fields, definition defaults, or the game default.
---@field graph MvGraphic|MvAnimation|MvImage|nil Graphic, animation or standalone image to draw. Set nil to clear.
---@field file MvGraphicLibrary|nil Library for contextual graph and animation selection.
---@field world MvCollisionWorld|nil World for contextual movement and collision queries.
---@field camera MvCamera|nil|false Camera to draw with. nil: main camera. false: screen space.
---@field region MvRegion|nil Drawing region descriptor.
---@field scroll MvScroll|nil Scroll descriptor.
---@field collider MvCollisionShape|nil|false Collision shape: `collision.rect`/`collision.circle` result, false to disable, nil for automatic.
---@field children_attached boolean Whether spawned children attach to this process. Default true.
---@field parent MvProcess|nil Read-only owner process.
---@field creator_id integer|nil Read-only creator process ID.
---@field priority integer Read-only scheduling priority.
---@field status MvProcessStatusName Read-only status.
---@field reason "returned"|"error"|"killed"|nil Read-only end reason after death.
---@field delta number Read-only logical seconds since the previous activation, including sleep and freeze ticks. Zero on the first activation.
---@field kill fun(self: MvProcess):boolean
---@field alive fun(self: MvProcess):boolean
---@field signal (fun(self: MvProcess, signal: "kill"):boolean)|(fun(self: MvProcess, signal: MvSignal))
---@field signal_tree fun(self: MvProcess, signal: MvSignal, options?: MvTreeSignalOptions)
---@field kill_tree fun(self: MvProcess, options?: MvTreeSignalOptions)
---@field send fun(self: MvProcess, message: any)
---@field recv fun(self: MvProcess):any
---@field children fun(self: MvProcess, options?: MvTreeQueryOptions):MvProcess[]
---@field descendants fun(self: MvProcess, options?: MvTreeQueryOptions):MvProcess[]
---@field collision fun(self: MvProcess, target: MvProcess|MvProcessConstructor):MvProcess|nil
---@field collisions fun(self: MvProcess, target: MvProcess|MvProcessConstructor):MvProcess[]
---@field set_priority fun(self: MvProcess, priority: integer)
---@field animation_elapsed fun(self: MvProcess):number|nil
---@field animation_finished fun(self: MvProcess):boolean
---@field [string] any Custom fields: processes accept any extra field.

---Options for `process()` (the second parameter of the definition call).
---@class MvProcessDefineOptions
---@field defaults table? Default field values copied into every spawned instance.
---@field children_attached boolean? Whether spawned children attach. Default true.

---Options for spawning: passed to the constructor returned by `process()`.
---@class MvSpawnOptions
---@field attached boolean? Attach the new process to its creator. Default true.

---Options for `start()`.
---@class MvStartOptions
---@field width integer? Initial window width in logical pixels. Default 800.
---@field height integer? Initial window height in logical pixels. Default 600.
---@field title string? Window title. Default "Malvado".
---@field fps integer? Default process cadence (0..60). Defaults to 0 (every tick).

---Game table returned by a declarative entry script.
---@class MvGame : MvStartOptions
---@field assets_dir? string Asset directory applied before init.
---@field init? fun() Called once before the first logical tick.

---Options for `children` and `descendants`.
---@class MvTreeQueryOptions
---@field relation? "ownership"|"creation" Tree relation. Default "ownership".
---@field type? MvProcessConstructor Filter by a process constructor.

---Options for `signal_tree` and `kill_tree`.
---@class MvTreeSignalOptions
---@field relation? "creation"|"ownership" Tree relation. Default "creation".

---Result table of `snapshot()`.
---@class MvSnapshotProcess
---@field id integer
---@field parent integer|nil
---@field creator_id integer|nil
---@field attached boolean
---@field type integer
---@field priority integer
---@field status MvLiveProcessStatusName
---@field messages integer
---@field draw_commands integer
---@field visible boolean

---@class MvSnapshot
---@field runtime string
---@field tick integer
---@field processes MvSnapshotProcess[]
---@field lineage {id: integer, creator_id: integer|nil, alive: boolean}[]

---Result of `world_move` and `world:move`.
---@class MvMoveResult
---@field hit boolean Whether the move hit a world rectangle.
---@field moved_x number Applied horizontal movement.
---@field moved_y number Applied vertical movement.
---@field normal_x number Collision normal x component.
---@field normal_y number Collision normal y component.
---@field id integer|nil ID of the hit world rectangle.

---Result of `world:sweep`.
---@class MvSweepResult
---@field hit boolean Whether the sweep hit a rectangle.
---@field id integer|nil ID of the hit rectangle.
---@field fraction number Fraction of the sweep completed.
---@field normal_x number Collision normal x component.
---@field normal_y number Collision normal y component.
