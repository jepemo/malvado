-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")

local Guard = mv.process(function()
    local state = "patrol"
    local x = 0
    local target = 4
    local rest_turns = 2
    print("Patrol")

    while true do
        if state == "patrol" then
            x = x + 1
            if target - x <= 2 then
                state = "chase"
                print("Chase from " .. x)
            end
        elseif state == "chase" then
            x = math.min(x + 2, target)
            if x >= target then
                state = "rest"
                print("Rest at " .. x)
            end
        elseif state == "rest" then
            rest_turns = rest_turns - 1
            if rest_turns == 0 then
                print("Guard recovered")
                return
            end
        end
        mv.frame()
    end
end)

return {init = function() Guard() end}
