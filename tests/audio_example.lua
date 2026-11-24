-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local frame = mv.frame
local play_sound = mv.play_sound
local play_music = mv.play_music
local ticks, effects, loops = 0, 0, 0
local sample, first

mv.frame = function()
  ticks = ticks + 1
  frame()
end
mv.key_pressed = function(key)
  return (key == "1" and ticks == 0)
    or (key == "2" and ticks == 1)
    or (key == "space" and ticks == 2)
end
-- Hold action keys after their initial edge: the demo must not replay them.
mv.key = function(key)
  return (key == "1" and ticks < 8)
    or (key == "2" and ticks >= 1 and ticks < 8)
    or (key == "space" and ticks >= 2 and ticks < 8)
    or (key == "escape" and ticks >= 8)
end
mv.play_sound = function(sound, options)
  effects = effects + 1
  local voice = assert(play_sound(sound, options))
  if effects == 1 then
    assert(options.volume == 0.3)
    sample, first = sound, voice
  else
    assert(sound == sample and voice ~= first and options.volume == 0.15)
  end
  return voice
end
mv.play_music = function(music)
  loops = loops + 1
  return play_music(music)
end

dofile("examples/audio/main.lua")
assert(effects == 2 and loops == 1 and ticks >= 8)
