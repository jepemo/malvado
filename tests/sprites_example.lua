-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local draw_image, frame = mv.draw_image, mv.frame
local frames, seen, unique = 0, {}, 0

mv.key = function(name) return name == "escape" and frames >= 8 end
mv.frame = function() frames = frames + 1; frame() end
mv.draw_image = function(image, ...)
  if not seen[image] then
    seen[image] = true
    unique = unique + 1
  end
  return draw_image(image, ...)
end

dofile("examples/sprites/main.lua")
assert(frames >= 8 and unique == 6)
