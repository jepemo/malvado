-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
mv.import_globals()
local events, a_id, b_id, c_id = {}, nil, nil, nil
local process = mv.process(function(self)
  for tick = 1, 3 do
    events[#events + 1] = self.name .. tick
    if self.name == "B" and tick == 1 then
      local nested = coroutine.create(function()
        local ok, message = pcall(set_priority, 20)
        assert(not ok and message:find("requires a running process", 1, true))
        mv.set_priority(c_id, -3) -- C retains its initial position in this tick.
      end)
      local ok, message = coroutine.resume(nested)
      assert(ok, message)
      assert(self.priority == 10)
      assert(not pcall(set_priority) and not pcall(set_priority, nil, 10))
      assert(not pcall(set_priority, self.id, 10) and not pcall(set_priority, self, 10, nil))
      for _, value in ipairs({"high", 1.5, math.huge, true, {}}) do
        assert(not pcall(set_priority, value) and not pcall(set_priority, self, value))
      end
      assert(not pcall(set_priority, nil) and self.priority == 10)
      mv.set_priority(math.mininteger)
      assert(self.priority == math.mininteger)
      set_priority(math.maxinteger)
      assert(self.priority == math.maxinteger)
      mv.set_priority(self, 0)
      assert(self.priority == 0 and self.status == "running")
      mv.process.set_priority(-10)
      assert(self.priority == -10)
    elseif self.name == "B" and tick == 2 then
      set_priority(1000)
      mv.set_priority(a_id, 1000) -- A wins the tie at the next tick.
    end
    mv.frame()
  end
end)
mv.start(function()
  local ok, message = pcall(mv.set_priority, 10)
  assert(not ok and message:find("requires a running process", 1, true))
  a_id = process { name = "A" }
  b_id = process { name = "B" }
  c_id = process { name = "C" }
  assert(mv.get_priority(a_id) == 0)
  mv.set_priority(a_id, math.maxinteger)
  assert(mv.get_priority(a_id) == math.maxinteger)
  mv.set_priority(a_id, math.mininteger)
  assert(mv.get_priority(a_id) == math.mininteger)
  mv.set_priority(a_id, 5)
  mv.set_priority(b_id, 10)
  mv.set_priority(c_id, 5)
  assert(not pcall(mv.set_priority, 99999, 1))
end)
assert(table.concat(events, ",") == "B1,A1,C1,A2,C2,B2,A3,B3,C3", table.concat(events, ","))
assert(not pcall(mv.get_priority, a_id))
assert(not pcall(mv.set_priority, a_id, 1))
