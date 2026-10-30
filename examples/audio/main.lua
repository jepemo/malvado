-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local sound, copy, music
local previous = {}

local function pressed(key)
  local down = mv.key(key)
  local result = down and not previous[key]
  previous[key] = down
  return result
end

local Demo = mv.process(function()
  local looping = false
  while not mv.key("escape") do
    if pressed("1") then mv.play_sound(sound) end
    if pressed("2") then mv.play_sound(copy) end
    if pressed("space") then
      looping = not looping
      if looping then mv.play_music(music) else mv.stop_music(music) end
    end
    if looping then mv.update_music(music) end
    mv.draw_text("1 / 2: play two independent effects", 30, 40, 22)
    mv.draw_text("Space: toggle loop  |  Escape: exit", 30, 80, 22)
    mv.draw_text(looping and "Loop: on" or "Loop: off", 30, 130, 22)
    mv.frame()
  end
  mv.stop_music(music)
  mv.stop_sound(sound)
  mv.stop_sound(copy)
end)

mv.start(function()
  sound = mv.load_sound("examples/audio/tone.wav")
  copy = mv.clone_sound(sound)
  music = mv.load_music("examples/audio/tone.wav", true)
  mv.sound_volume(sound, 0.3)
  mv.sound_volume(copy, 0.3)
  mv.music_volume(music, 0.2)
  mv.clear_screen(25, 30, 45)
  Demo()
end, {width = 640, height = 240, title = "Audio demo"})
