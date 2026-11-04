-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local original_frame, original_process = mv.frame, mv.process
local ticks, last_x, player = 0, nil, nil
mv.process = function(fn, options)
  return original_process(function(self)
    player = self
    return fn(self)
  end, options)
end
mv.key = function(name)
  if name == "right" then return true end
  if name == "escape" then return ticks >= 54 end
  return false
end
mv.frame = function()
  ticks = ticks + 1
  last_x = player.x
  assert(last_x <= 284)
  return original_frame()
end
dofile("examples/world-collision/main.lua")
assert(ticks == 54 and last_x == 284)
