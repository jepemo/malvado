-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local wav = "fixtures/silence.wav"
local sound = mv.load_sound(wav)
assert(not pcall(mv.load_sound, "fixtures/missing.wav"))
assert(not pcall(mv.load_sound, "fixtures/silence.mp3"))

for _, control in ipairs {mv.stop_sound, mv.pause_sound, mv.resume_sound,
                           mv.sound_playing, mv.sound_volume} do
  local ok, message = pcall(control, sound, 0.5)
  assert(not ok and message:match("voice expected"), message)
end
for _, options in ipairs {true, "volume", {0.5}, {pitch = 1}, {volume = "0.5"},
                           {volume = false}, {volume = -0.1}, {volume = 1.1},
                           {volume = 0 / 0}, {volume = math.huge}} do
  assert(not pcall(mv.play_sound, sound, options))
end

-- Exercise Lua allocation and native clone, volume and start failures.
local failures = {"not enough memory", "could not create voice", "could not play", "could not play"}
assert(sound.play == mv.play_sound and sound.stop == nil)
assert(not pcall(sound.play, nil) and not pcall(sound.play, sound, {volume = 1.1}))
local play = sound.play
for mode, expected in ipairs(failures) do
  test_audio_failure(mode)
  local ok, message = pcall(play, sound)
  test_audio_failure(0)
  assert(not ok and message:match(expected), message)
  assert(test_voice_state() == 0)
end

local first = assert(sound:play())
local count, volume = test_voice_state()
assert(count == 1 and volume == 1)
local second = assert(mv.play_sound(sound, {volume = 0.75}))
count, volume = test_voice_state()
assert(count == 2 and volume == 0.75)
assert(type(first) == "userdata" and first ~= second)
assert(first:is_playing() == mv.sound_playing(first) and first.is_playing(first))
for method, procedure in pairs {
  stop = mv.stop_sound, pause = mv.pause_sound, resume = mv.resume_sound,
  is_playing = mv.sound_playing, set_volume = mv.sound_volume,
} do
  assert(first[method] == procedure and second[method] == procedure)
  assert(not pcall(first[method], sound, 0.5))
end
assert(select("#", first:pause()) == 0)
assert(not first:is_playing() and second:is_playing())
assert(select("#", first:set_volume(0.25)) == 0)
assert(select("#", first:resume()) == 0)
assert(first:is_playing())
assert(mv.sound_playing(first) and mv.sound_playing(second))
mv.pause_sound(first)
mv.pause_sound(first)
assert(not mv.sound_playing(first) and mv.sound_playing(second))
assert(not pcall(mv.sound_volume, first))
assert(not pcall(mv.sound_volume, first, "0.5"))
assert(not pcall(mv.sound_volume, first, 0 / 0))
mv.sound_volume(first, 0)
mv.sound_volume(second, 1)
mv.resume_sound(first)
assert(mv.sound_playing(first))
assert(select("#", second:stop()) == 0)
assert(select("#", second.stop(second)) == 0)
assert(mv.sound_playing(first) and not mv.sound_playing(second))
for _, control in ipairs {mv.pause_sound, mv.resume_sound, mv.sound_volume} do
  local ok, message = pcall(control, second, 0.5)
  assert(not ok and message:match("finished"), message)
end
mv.stop_sound(first)
assert(test_voice_state() == 0)

local voices = {}
for i = 1, 32 do
  local voice = assert(mv.play_sound(sound, {}))
  mv.pause_sound(voice)
  voices[i] = voice
end
assert(mv.play_sound(sound) == nil and sound:play() == nil)
test_audio_failure(1)
local overflow_ok, overflow = pcall(play, sound)
test_audio_failure(0)
assert(overflow_ok and overflow == nil)
assert(not pcall(mv.play_sound, sound, {volume = "0.5"}))
for _, voice in ipairs(voices) do
  assert(not mv.sound_playing(voice))
  mv.resume_sound(voice)
  assert(mv.sound_playing(voice))
  mv.pause_sound(voice)
end
mv.stop_sound(voices[1])
local replacement = assert(mv.play_sound(sound, {volume = 1}))
assert(not mv.sound_playing(voices[1]) and mv.sound_playing(replacement))
mv.stop_sound(voices[1]) -- The old handle cannot affect the reused slot.
for i = 2, #voices do mv.stop_sound(voices[i]) end

local ogg = mv.load_music("fixtures/silence.ogg")
mv.play_music(ogg)
mv.update_music(ogg)
mv.stop_music(ogg)
assert(not pcall(mv.load_music, "fixtures/missing.wav"))
for _, options in ipairs {true, "yes", {loop = "yes"}, {loop = 1}, {looping = true}, {false}} do
  assert(not pcall(mv.load_music, wav, options))
end
local music = mv.load_music(wav, {loop = true})
for method, procedure in pairs {
  play = mv.play_music, update = mv.update_music, stop = mv.stop_music,
  pause = mv.pause_music, resume = mv.resume_music,
  is_playing = mv.music_playing, set_volume = mv.music_volume,
} do
  assert(music[method] == procedure and ogg[method] == procedure)
  assert(not pcall(music[method], nil, 0.5))
end
assert(select("#", music:set_volume(0.5)) == 0)
assert(select("#", music:play()) == 0)
assert(select("#", music:update()) == 0)
assert(select("#", music:pause()) == 0)
assert(not music:is_playing() and music.is_playing(music) == mv.music_playing(music))
assert(select("#", music:resume()) == 0)
assert(select("#", music:stop()) == 0)
assert(not music:is_playing())
assert(not pcall(mv.music_volume, music, -1))
mv.music_volume(music, 0.5)
mv.play_music(music)
mv.update_music(music)
mv.pause_music(music)
assert(not mv.music_playing(music))
mv.resume_music(music)
mv.stop_music(music)
assert(not mv.music_playing(music))

-- Both unobserved playback and paused playback are released on shutdown.
mv.play_sound(sound)
mv.pause_sound(assert(mv.play_sound(sound, {volume = 0})))
