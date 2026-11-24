-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0
--
-- Audio types for the Lua Language Server (LuaLS).

---@meta

---Reusable WAV sample loaded with `load_sound`.
---@class MvSound
---@field play fun(self: MvSound, options?: MvSoundPlayOptions):MvVoice|nil

---One playback started with `play_sound`.
---@class MvVoice
---@field stop fun(self: MvVoice)
---@field pause fun(self: MvVoice)
---@field resume fun(self: MvVoice)
---@field is_playing fun(self: MvVoice):boolean
---@field set_volume fun(self: MvVoice, volume: number)

---@class MvSoundPlayOptions
---@field volume? number Initial volume from 0 to 1. Defaults to 1.

---Music stream loaded with `load_music`.
---@class MvMusic
---@field play fun(self: MvMusic)
---@field update fun(self: MvMusic)
---@field stop fun(self: MvMusic)
---@field pause fun(self: MvMusic)
---@field resume fun(self: MvMusic)
---@field is_playing fun(self: MvMusic):boolean
---@field set_volume fun(self: MvMusic, volume: number)

---@class MvMusicLoadOptions
---@field loop? boolean Repeat the stream. Defaults to false.
