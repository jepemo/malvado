-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local original_frame = mv.frame
local original_animation = mv.animation_frame
local original_load = mv.load_asset
local define = mv.process
local actor, loaded
local ticks, seen = 0, {}
mv.load_asset = function(...)
  local atlas = original_load(...)
  loaded = atlas
  for id = 1, 3 do
    local info = mv.sprite_info(atlas, id)
    assert(info.source_width == 8 and info.source_height == 8)
    assert(info.trim_x == 2 and info.trim_y == 2)
    assert(info.pivot_x == 2 and info.pivot_y == 2)
  end
  assert(original_animation(atlas, "bounce", 99) == 1)
  assert(original_animation(atlas, "bounce", 100) == 2)
  assert(original_animation(atlas, "bounce", 300) == 3)
  return atlas
end
mv.process = function(fn, options)
  return define(function(self) actor = self; return fn(self) end, options)
end
mv.key = function(name) return name == "escape" and ticks >= 80 end
mv.frame = function()
  local elapsed = mv.graphics.animation_elapsed(actor)
  assert(elapsed ~= nil and actor.graph == mv.animation(loaded, "bounce"))
  seen[original_animation(loaded, "bounce", math.floor(elapsed))] = true
  ticks = ticks + 1
  if ticks == 80 then assert(seen[1] and seen[2] and seen[3]) end
  return original_frame()
end
return dofile("examples/aseprite/main.lua")
