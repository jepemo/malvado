-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local change_text, draw_image = mv.change_text, mv.draw_image
local ticks, draws, count_seen = 0, 0, false

mv.key = function(key)
  if key == "escape" then
    ticks = ticks + 1
    return ticks > 5
  end
  return key == "space" and ticks <= 3
end
mv.change_text = function(id, value, ...)
  if value == "Processes: 3" then count_seen = true end
  return change_text(id, value, ...)
end
mv.draw_image = function(...)
  draws = draws + 1
  return draw_image(...)
end

dofile("examples/bench/main.lua")
assert(count_seen and draws > 0)
