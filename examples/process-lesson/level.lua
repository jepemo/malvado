-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")

local Enemy = mv.process(function()
    while true do mv.frame() end
end)

local Music = mv.process(function()
    while true do mv.frame() end
end)

local enemy, music
local Level = mv.process(function()
    enemy = Enemy()
    music = mv.process.spawn(Music, {}, {attached = false})
    mv.frame()
    print("Level complete")
end)

local Controller = mv.process(function()
    local level = Level()
    while mv.process.alive(level) do mv.frame() end
    print("Level: " .. level.reason)
    print("Enemy: " .. enemy.status)
    print("Music: " .. music.status)
    mv.kill(music)
    print("Music: " .. music.reason)
end)

return {init = function() Controller() end}
