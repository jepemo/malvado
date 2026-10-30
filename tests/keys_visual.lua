-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local names = {"left", "right", "up", "down", "a", "0", "space"}
local process = mv.process(function()
    local phase = 0
    for _ = 1, 600 do
        local held = 0
        for _, name in ipairs(names) do
            if mv.key(name) then held = held + 1 end
        end
        if phase == 0 and held == #names then
            phase = 1
        elseif phase == 1 and held == 0 then
            phase = 2
        elseif phase == 2 and mv.key("escape") then
            return
        end
        mv.frame()
    end
    error("keyboard input test timed out")
end)
mv.start(function() process() end, {width = 160, height = 120, title = "Malvado Lua keys"})
