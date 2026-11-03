-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
assert(process == nil and frame == nil and start == nil)
mv.unlisted_extension = function() end
KEY_UP = "occupied"
local constants_ok = pcall(mv.import_globals)
assert(not constants_ok and process == nil and KEY_UP == "occupied")
KEY_UP = nil
key = "occupied"
local ok, message = pcall(mv.import_globals)
assert(not ok and message:find("global already exists: key", 1, true))
assert(process == nil and frame == nil and start == nil and key == "occupied")
key = nil
mv.import_globals()
mv.import_globals()
assert(process == mv.process and frame == mv.frame and start == mv.start)
assert(key == mv.key and draw_text == mv.draw_text)
assert(KEY_UP == mv.KEY_UP and KEY_UP == "up")
assert(KEY_A == "a" and KEY_0 == "0" and SIGNAL_KILL == "kill")
assert(mv.key(KEY_UP) == mv.key("up"))
assert(legacy_fpg == nil and import_globals == nil and unlisted_extension == nil)

local completed = false
local task = process(function()
  frame()
  completed = true
end)
start(function() task() end)
assert(completed)
