-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
return function()
  assert(not pcall(mv.load_sound, "missing.wav"))
  assert(not pcall(mv.load_sound, "empty.wav"))
  assert(not pcall(mv.load_music, "empty.wav"))
  local original = mv.load_sound("silence.wav")
  assert(not pcall(mv.sound_volume, original, -1))
  assert(not pcall(mv.sound_volume, original, 0 / 0))
  local sound = mv.clone_sound(original)
  original = nil
  collectgarbage("collect")
  local music = mv.load_music("silence.ogg", true)
  mv.play_sound(sound)
  mv.pause_sound(sound)
  mv.play_music(music)
  mv.pause_music(music)
  local finite
  if not os.getenv("MALVADO_APK_LIFECYCLE") then
    finite = mv.load_sound("silence.wav")
    mv.play_sound(finite)
  end
  local weak = setmetatable({sound, music}, {__mode = "v"})
  sound, music = nil, nil
  return {
    frame = function()
      collectgarbage("collect")
      local sound, music = assert(weak[1]), assert(weak[2])
      assert(not mv.sound_playing(sound) and not mv.music_playing(music))
      mv.update_music(music)
      assert(not mv.music_playing(music))
    end,
    finish = function()
      local sound, music = assert(weak[1]), assert(weak[2])
      if finite then assert(not mv.sound_playing(finite)) end
      mv.resume_sound(sound)
      mv.resume_music(music)
      assert(mv.sound_playing(sound) and mv.music_playing(music))
      mv.stop_sound(sound)
      mv.stop_music(music)
      assert(not mv.sound_playing(sound) and not mv.music_playing(music))
      -- Leave paused playback roots for runner shutdown to release.
      mv.play_sound(sound)
      mv.pause_sound(sound)
      mv.play_music(music)
      mv.pause_music(music)
      local file = assert(io.open("audio.txt", "ab"))
      assert(file:write("1"))
      assert(file:close())
    end,
  }
end
