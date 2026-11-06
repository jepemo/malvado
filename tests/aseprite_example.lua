-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local original_frame = mv.frame
local original_animation = mv.animation_frame
local ticks, seen = 0, {}
mv.key = function(name) return name == "escape" and ticks >= 80 end
mv.animation_frame = function(...)
  local id = original_animation(...)
  seen[id] = true
  return id
end
mv.frame = function()
  ticks = ticks + 1
  if ticks == 80 then assert(seen[1] and seen[2] and seen[3]) end
  return original_frame()
end
return dofile("examples/aseprite/main.lua")
