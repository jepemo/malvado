-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local events = {}
local task = mv.process(function()
  assert(events[1] == "init")
  print("declarative tick")
end)

return {
  width = 320,
  height = 240,
  init = function()
    events[1] = "init"
    task()
    assert(not pcall(mv.start, function() end))
  end,
}
