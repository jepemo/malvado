-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local original_frame, original_draw = mv.frame, mv.draw_fcircle
local ticks, last_x = 0, nil
mv.key = function(name)
  if name == "right" then return true end
  if name == "escape" then return ticks >= 54 end
  return false
end
mv.frame = function()
  ticks = ticks + 1
  return original_frame()
end
mv.draw_fcircle = function(x, y, radius)
  last_x = x
  assert(x <= 284)
  return original_draw(x, y, radius)
end
dofile("examples/world-collision/main.lua")
assert(ticks == 54 and last_x == 284)
