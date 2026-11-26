-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")

local Controller = mv.process(function()
    local mode = "menu"
    local remaining = 2
    print(mode)

    -- A scripted input sequence makes the example runnable in a terminal.
    for _, action in ipairs({"start", "tick", "pause", "tick", "resume", "tick"}) do
        if mode == "menu" and action == "start" then
            mode = "play"
            print(mode)
        elseif mode == "play" and action == "pause" then
            mode = "pause"
            print(mode)
        elseif mode == "pause" and action == "resume" then
            mode = "play"
            print(mode)
        elseif mode == "play" and action == "tick" then
            remaining = remaining - 1
            print("Time left: " .. remaining)
            if remaining == 0 then
                mode = "game over"
                print(mode)
                return
            end
        end
        mv.frame()
    end
end)

return {init = function() Controller() end}
