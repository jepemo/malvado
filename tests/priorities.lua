-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local events, a_id, b_id, c_id = {}, nil, nil, nil
local process = mv.process(function(self)
  for tick = 1, 3 do
    events[#events + 1] = self.name .. tick
    if self.name == "B" and tick == 1 then
      mv.set_priority(c_id, -3) -- C retains its initial position in this tick.
    elseif self.name == "B" and tick == 2 then
      mv.set_priority(a_id, 1000) -- A moves ahead at the next tick.
    end
    mv.frame()
  end
end)
mv.start(function()
  a_id = process { name = "A" }
  b_id = process { name = "B" }
  c_id = process { name = "C" }
  assert(mv.get_priority(a_id) == 0)
  mv.set_priority(a_id, 5)
  mv.set_priority(b_id, 10)
  mv.set_priority(c_id, 5)
  assert(not pcall(mv.set_priority, 99999, 1))
end)
assert(table.concat(events, ",") == "B1,A1,C1,B2,A2,C2,A3,B3,C3", table.concat(events, ","))
assert(not pcall(mv.get_priority, a_id))
