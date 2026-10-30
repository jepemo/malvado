-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local sound = mv.load_sound("tests/fixtures/silence.wav")
assert(not pcall(mv.load_sound, "tests/fixtures/missing.wav"))
assert(not pcall(mv.load_sound, "tests/fixtures/silence.mp3"))
assert(not pcall(mv.sound_volume, sound, -0.1))
assert(not pcall(mv.sound_volume, sound, 1.1))
assert(not pcall(mv.sound_volume, sound, 0 / 0))
mv.sound_volume(sound, 0.5)
mv.play_sound(sound)
mv.stop_sound(sound)
assert(not mv.sound_playing(sound))
sound = nil
collectgarbage("collect")
local live_until_close = mv.load_sound("tests/fixtures/silence.wav")
mv.play_sound(live_until_close)
