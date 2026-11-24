-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
return function()
  assert(not pcall(mv.load_sound, "missing.wav"))
  assert(not pcall(mv.load_sound, "empty.wav"))
  assert(not pcall(mv.load_music, "empty.wav"))
  local sound = mv.load_sound("silence.wav")
  assert(not pcall(mv.play_sound, sound, {volume = -1}))
  assert(not pcall(mv.play_sound, sound, {volume = 0 / 0}))
  local voice = assert(mv.play_sound(sound))
  mv.pause_sound(voice)
  local music = mv.load_music("silence.ogg", {loop = true})
  mv.play_music(music)
  mv.pause_music(music)
  local finite
  if not os.getenv("MALVADO_APK_LIFECYCLE") then
    finite = assert(mv.play_sound(sound))
  end
  local weak = setmetatable({voice, music}, {__mode = "v"})
  voice, music = nil, nil
  return {
    frame = function()
      collectgarbage("collect")
      local voice, music = assert(weak[1]), assert(weak[2])
      assert(not mv.sound_playing(voice) and not mv.music_playing(music))
      mv.update_music(music)
      assert(not mv.music_playing(music))
    end,
    finish = function()
      local voice, music = assert(weak[1]), assert(weak[2])
      if finite then assert(not mv.sound_playing(finite)) end
      mv.resume_sound(voice)
      mv.resume_music(music)
      assert(mv.sound_playing(voice) and mv.music_playing(music))
      mv.stop_sound(voice)
      mv.stop_music(music)
      assert(not mv.sound_playing(voice) and not mv.music_playing(music))
      -- Leave paused playback roots for runner shutdown to release.
      mv.pause_sound(assert(mv.play_sound(sound)))
      mv.play_music(music)
      mv.pause_music(music)
      local file = assert(io.open("audio.txt", "ab"))
      assert(file:write("1"))
      assert(file:close())
    end,
  }
end
