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
    mv.frame()
  end
end)
local odd = mv.process(function()
  for _ = 1, 5 do
    fractional[#fractional + 1] = mv.snapshot().tick
    mv.frame()
  end
end)
local fast = mv.process(function(self)
  assert(self.fps == 0)
  self.fps = self.fps
  changed[#changed + 1] = mv.snapshot().tick
  self.fps = 60
  mv.frame()
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
