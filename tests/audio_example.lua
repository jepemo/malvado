-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local frame = mv.frame
local play_sound = mv.play_sound
local play_music = mv.play_music
local ticks, effects, loops = 0, 0, 0

mv.frame = function()
  ticks = ticks + 1
  frame()
end
mv.key = function(key)
  return (key == "1" and ticks == 0)
    or (key == "2" and ticks == 1)
    or (key == "space" and ticks == 2)
    or (key == "escape" and ticks >= 8)
end
mv.play_sound = function(sound)
  effects = effects + 1
  return play_sound(sound)
end
mv.play_music = function(music)
  loops = loops + 1
  return play_music(music)
end

dofile("examples/audio/main.lua")
assert(effects == 2 and loops == 1 and ticks >= 8)
