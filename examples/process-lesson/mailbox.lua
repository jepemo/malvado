-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")

local Scoreboard = mv.process(function()
    local score = 0
    while true do
        local message = mv.recv()
        if message == false then
            print("Final score: " .. score)
            return
        elseif message ~= nil then
            score = score + message
            print("Score: " .. score)
        end
        mv.frame()
    end
end)

local Player = mv.process(function(self)
    mv.send(self.scoreboard, 10)
    mv.send(self.scoreboard, 5)
    mv.send(self.scoreboard, false) -- End-of-round message.
end)

return {
    init = function()
        local scoreboard = Scoreboard()
        Player {scoreboard = scoreboard}
    end,
}
