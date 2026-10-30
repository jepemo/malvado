-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

require("malvado").import_globals()
local sound, copy, music
local previous = {}

local function pressed(name)
  local down = key(name)
  local result = down and not previous[name]
  previous[name] = down
  return result
end

local Demo = process(function()
  local looping = false
  while not key("escape") do
    if pressed("1") then play_sound(sound) end
    if pressed("2") then play_sound(copy) end
    if pressed("space") then
      looping = not looping
      if looping then play_music(music) else stop_music(music) end
    end
    if looping then update_music(music) end
    draw_text("1 / 2: play two independent effects", 30, 40, 22)
    draw_text("Space: toggle loop  |  Escape: exit", 30, 80, 22)
    draw_text(looping and "Loop: on" or "Loop: off", 30, 130, 22)
    frame()
  end
  stop_music(music)
  stop_sound(sound)
  stop_sound(copy)
end)

start(function()
  sound = load_sound("examples/audio/tone.wav")
  copy = clone_sound(sound)
  music = load_music("examples/audio/tone.wav", true)
  sound_volume(sound, 0.3)
  sound_volume(copy, 0.3)
  music_volume(music, 0.2)
  clear_screen(25, 30, 45)
  Demo()
end, {width = 640, height = 240, title = "Audio demo"})
