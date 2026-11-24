-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0
--
-- Audio types for the Lua Language Server (LuaLS).

---@meta

---Reusable WAV sample loaded with `load_sound`.
---@class MvSound

---One playback started with `play_sound`.
---@class MvVoice

---@class MvSoundPlayOptions
---@field volume? number Initial volume from 0 to 1. Defaults to 1.

---Music stream loaded with `load_music`.
---@class MvMusic

---@class MvMusicLoadOptions
---@field loop? boolean Repeat the stream. Defaults to false.
