-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

require("malvado").import_globals()
local sound, music
local Demo = process(function()
  local looping = false
  while not key(KEY_ESCAPE) do
    if key_pressed(KEY_1) then play_sound(sound, {volume = 0.3}) end
    if key_pressed(KEY_2) then play_sound(sound, {volume = 0.15}) end
    if key_pressed(KEY_SPACE) then
      looping = not looping
      if looping then play_music(music) else stop_music(music) end
    end
    draw_text {text = "1 / 2: play overlapping tones", x = 30, y = 40, size = 22}
    draw_text {text = "Space: toggle loop  |  Escape: exit", x = 30, y = 80, size = 22}
    draw_text {text = looping and "Loop: on" or "Loop: off", x = 30, y = 130, size = 22}
    frame()
  end
  stop_music(music)
end)

start(function()
  sound = load_sound("tone.wav")
  music = load_music("tone.wav", {loop = true})
  music_volume(music, 0.2)
  clear_screen {25, 30, 45, 255}
  Demo()
end, {width = 640, height = 240, title = "Audio demo"})
