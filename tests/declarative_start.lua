-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local module = require("tests.declarative_module")
assert(type(module.init) == "function")
local events = {}
local task = mv.process(function()
  assert(events[1] == "init")
  print("declarative tick")
end)

return {
  width = 320,
  height = 240,
  init = function(...)
    assert(select("#", ...) == 0)
    assert(#events == 0)
    local width, height = mv.window_size()
    assert(width == 320 and height == 240)
    events[1] = "init"
    task()
    assert(not pcall(mv.start, function() end))
  end,
}
