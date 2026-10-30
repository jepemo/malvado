-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local regular, fractional, changed = {}, {}, {}
local slow = mv.process(function(self)
  assert(self.fps == 15)
  assert(not pcall(function() self.fps = -1 end))
  assert(not pcall(function() self.fps = 61 end))
  for _ = 1, 5 do
    regular[#regular + 1] = mv.snapshot().tick
    local expected = #regular == 1 and 0 or 4 / 60
    assert(math.abs(self.delta - expected) < 1e-12)
    mv.frame()
  end
end)
local odd = mv.process(function(self)
  local previous
  for _ = 1, 5 do
    local tick = mv.snapshot().tick
    fractional[#fractional + 1] = tick
    local expected = previous and (tick - previous) / 60 or 0
    assert(math.abs(self.delta - expected) < 1e-12)
    previous = tick
    mv.frame()
  end
end)
local fast = mv.process(function(self)
  assert(self.fps == 0)
  self.fps = self.fps
  assert(self.delta == 0 and not pcall(function() self.delta = 1 end))
  changed[#changed + 1] = mv.snapshot().tick
  self.fps = 60
  mv.frame()
  assert(math.abs(self.delta - 1 / 60) < 1e-12)
  changed[#changed + 1] = mv.snapshot().tick
end)
mv.start(function()
  assert(not pcall(slow, {fps = 61}))
  slow {fps = 15}
  odd {fps = 17}
  fast()
end)
assert(table.concat(regular, ",") == "1,5,9,13,17", table.concat(regular, ","))
assert(table.concat(fractional, ",") == "1,5,9,12,16", table.concat(fractional, ","))
assert(table.concat(changed, ",") == "1,2", table.concat(changed, ","))
