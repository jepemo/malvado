-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local counts, handles = {}, {}
local worker = mv.process(function(self)
  while true do
    counts[self.fps] = counts[self.fps] + 1
    mv.frame()
  end
end)
local controller = mv.process(function()
  for _ = 1, 60 do mv.frame() end
  for fps = 0, 60 do
    local expected = fps == 0 and 61 or fps + 1
    assert(counts[fps] == expected,
      string.format("fps %d: expected %d activations, got %d", fps, expected, counts[fps]))
    mv.kill(handles[fps])
  end
end)
mv.start(function()
  for fps = 0, 60 do
    counts[fps] = 0
    handles[fps] = worker {fps = fps}
  end
  controller()
end)
