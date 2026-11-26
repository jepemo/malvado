-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")

local show = mv.process(function()
    local sample = mv.load_sound("tone.wav")
    local voice = assert(sample:play {volume = 0.2})
    assert(voice:is_playing())
    voice:pause()
    assert(not voice:is_playing())
    voice:set_volume(0.15)
    voice:resume()
    for _ = 1, 8 do mv.frame() end
    voice:stop()
    assert(not voice:is_playing())
    print("Sound voice stopped")

    local track = mv.load_music("tone.wav", {loop = true})
    track:set_volume(0.1)
    track:play()
    track:update()
    assert(track:is_playing())
    track:pause()
    assert(not track:is_playing())
    track:resume()
    for _ = 1, 75 do mv.frame() end
    track:stop()
    assert(not track:is_playing())
    print("Music stream stopped")
end)

mv.start(function() show() end, {title = "Sound and music", width = 320, height = 120})
