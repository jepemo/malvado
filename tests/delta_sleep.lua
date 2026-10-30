-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local worker_id
local worker = mv.process(function(self)
  assert(self.delta == 0)
  mv.signal(self.id, "sleep")
  mv.frame()
  assert(mv.snapshot().tick == 6)
  assert(math.abs(self.delta - 5 / 60) < 1e-12)
end)
local controller = mv.process(function()
  while mv.snapshot().tick < 5 do mv.frame() end
  mv.signal(worker_id, "wakeup")
end)
mv.start(function()
  worker_id = worker()
  controller()
end)
