-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local ok, message = pcall(mv.load_sound, "tests/fixtures/silence.wav")
assert(not ok and message:match("headless"))
assert(not pcall(mv.load_sound, "bad\0name.wav"))
assert(not pcall(mv.play_sound, {}))
assert(not pcall(mv.stop_sound, {}))
assert(not pcall(mv.sound_playing, {}))
assert(not pcall(mv.sound_volume, {}, 1))
assert(not pcall(mv.load_music, "tests/fixtures/silence.wav", true))
assert(not pcall(mv.load_music, "bad\0name.wav"))
assert(not pcall(mv.play_music, {}))
assert(not pcall(mv.update_music, {}))
