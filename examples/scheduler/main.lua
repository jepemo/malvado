-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

require("malvado").import_globals()
local worker = process(function(self)
  print(self.name .. ": first tick")
  if self.name == "Low" then
    -- Move ahead of the other worker on the next tick.
    set_priority(20)
  end
  frame()
  print(self.name .. ": second tick")
end)
start(function()
  local low = worker { name = "Low" }
  local high = worker { name = "High" }
  set_priority(low, -10)
  set_priority(high, 10)
end)
