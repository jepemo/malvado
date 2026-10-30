-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local names = {"left", "middle", "right"}
local process = mv.process(function()
    local phase = 0
    for _ = 1, 600 do
        local x, y = mv.mouse_position()
        local held = 0
        for _, name in ipairs(names) do
            if mv.mouse_button(name) then held = held + 1 end
        end
        if phase == 0 and x == 50 and y == 45 and held == 3 then
            phase = 1
        elseif phase == 1 and held == 0 then
            return
        end
        mv.frame()
    end
    error("mouse input test timed out")
end)
mv.start(function() process() end, {width = 160, height = 120, title = "Malvado Lua mouse"})
