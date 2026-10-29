-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local worker = mv.process(function(self)
  print(self.name .. ": first tick")
  mv.frame()
  print(self.name .. ": second tick")
end)
mv.start(function()
  local low = worker { name = "Low" }
  local high = worker { name = "High" }
  mv.set_priority(low, -10)
  mv.set_priority(high, 10)
end)
