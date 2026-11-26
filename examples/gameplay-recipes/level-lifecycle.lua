-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")

local Effect = mv.process(function(self)
    local actors = self.actors
    -- A probe lets the controller check that cancellation closed the effect.
    local cleanup <close> = setmetatable({}, {
        __close = function() actors.effect_closed = true end,
    })
    actors.effect_started = true
    while true do mv.frame() end
end)

local Enemy = mv.process(function(self)
    self.actors.effect = Effect {actors = self.actors}
    while true do mv.frame() end
end)

local Ambience = mv.process(function()
    while true do mv.frame() end
end)

local Level = mv.process(function(self)
    local actors = self.actors
    actors.enemy = Enemy {actors = actors}
    actors.ambience = mv.process.spawn(Ambience, {}, {attached = false})

    -- Let the effect start so it has something to close at teardown.
    while not actors.effect_started do mv.frame() end
    assert(mv.process.alive(actors.enemy) and mv.process.alive(actors.effect))
    assert(actors.enemy.parent.id == self.id)
    assert(actors.effect.parent.id == actors.enemy.id)
    assert(mv.process.alive(actors.ambience) and actors.ambience.parent == nil)
    assert(actors.ambience.creator_id == self.id)
end)

local HUD = mv.process(function(self)
    local score = 0
    while true do
        local points = mv.recv()
        if points ~= nil then
            score = score + points
            print("HUD score: " .. score)
            mv.send(self.parent, score)
        end
        mv.frame()
    end
end)

local Controller = mv.process(function()
    local hud = HUD()
    for number = 1, 2 do
        local actors = {}
        local level = Level {actors = actors}
        while mv.process.alive(level) do mv.frame() end

        assert(level.reason == "returned")
        assert(not mv.process.alive(actors.enemy) and not mv.process.alive(actors.effect))
        assert(actors.effect_closed)
        assert(mv.process.alive(hud) and mv.process.alive(actors.ambience))
        assert(mv.kill(actors.ambience))
        assert(not mv.process.alive(actors.ambience))
        print("Level " .. number .. " cleared")

        mv.send(hud, 10)
        local score
        repeat
            mv.frame()
            score = mv.recv()
        until score ~= nil
        assert(score == number * 10)
    end
    -- Returning ends the attached HUD with the controller.
end)

return {init = function() Controller() end}
