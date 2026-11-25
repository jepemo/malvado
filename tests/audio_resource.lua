-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local wav = "fixtures/silence.wav"
local ogg = "fixtures/silence.ogg"
local weak = setmetatable({}, {__mode = "v"})
local retained, paused = nil, {}
local producer = mv.process(function()
  local sound = mv.load_sound(wav)
  local music = mv.load_music(ogg, {loop = true})
  assert(not pcall(mv.music_volume, music, "0.5"))
  assert(not pcall(music.set_volume, music, 0 / 0))
  assert(not pcall(music.set_volume, music, 0.5, nil))
  music:set_volume(0.5)
  local voice = assert(mv.play_sound(sound))
  assert(not pcall(mv.stop_sound, voice, nil))
  assert(not pcall(voice.set_volume, voice, "0.5"))
  voice:set_volume(0.5)
  mv.pause_sound(voice)
  weak.sample, weak.voice, weak.music = sound, voice, music
  mv.play_music(music)
  mv.pause_music(music)

  -- The runner must complete this stream without an explicit update.
  local finite = mv.load_music(wav)
  weak.finite = finite
  mv.play_music(finite)
  local loop = mv.load_music(ogg, {loop = true})
  weak.loop = loop
  mv.play_music(loop)

  for i = 1, 29 do
    local hold = assert(mv.play_sound(sound))
    mv.pause_sound(hold)
    paused[i] = hold
  end
  weak.natural = assert(mv.play_sound(sound))
  retained = assert(mv.play_sound(sound))
  assert(mv.play_sound(sound) == nil)
end)
local controller = mv.process(function()
  mv.frame()
  -- The native harness pauses the scheduler until the finite audio ends.
  -- Collection here runs before the next scheduler maintenance pass.
  collectgarbage("collect")
  assert(weak.sample == nil and weak.voice and weak.music)
  assert(weak.natural == nil and weak.finite == nil)
  assert(weak.loop and mv.music_playing(weak.loop))
  assert(test_voice_state() == 30) -- The retained handle no longer owns an alias.

  local new_sample = mv.load_sound(wav)
  local first = assert(mv.play_sound(new_sample))
  local second = assert(mv.play_sound(new_sample))
  assert(mv.play_sound(new_sample) == nil)
  assert(not mv.sound_playing(retained))
  assert(not pcall(mv.resume_sound, retained))
  mv.stop_sound(retained)
  assert(mv.sound_playing(first) and mv.sound_playing(second))
  mv.stop_sound(first)
  mv.stop_sound(second)

  mv.resume_sound(weak.voice)
  assert(mv.sound_playing(weak.voice))
  mv.stop_sound(weak.voice)
  mv.resume_music(weak.music)
  mv.stop_music(weak.music)
  mv.stop_music(weak.loop)
  collectgarbage("collect")
  assert(weak.voice == nil and weak.music == nil and weak.loop == nil)

  -- Leave paused voices and looping music for runner shutdown.
  local music = mv.load_music(ogg, {loop = true})
  mv.play_music(music)
end)
mv.start(function()
  controller()
  producer()
end, {width = 64, height = 64})
