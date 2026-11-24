-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
mv.import_globals()
assert(set_window_size == mv.set_window_size)
local module = require("tests.declarative_module")
assert(type(module.init) == "function")
local events = {}
local task = mv.process(function(self)
  assert(self.fps == 0)
  assert(events[1] == "init")
  local width, height = window_size()
  assert(width == 480 and height == 270)
  print("declarative tick")
end)

local game = {
  width = 320,
  height = 240,
}
function game.init(...)
  assert(select("#", ...) == 0)
  assert(#events == 0)
  local width, height = mv.window_size()
  assert(width == 320 and height == 240)
  game.width, game.height = 1, 1
  width, height = window_size()
  assert(width == 320 and height == 240)
  set_window_size(480, 270)
  events[1] = "init"
  task()
  assert(not pcall(mv.start, function() end))
end
return game
