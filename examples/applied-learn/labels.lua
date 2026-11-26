-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local center = 160

local scoreboard = mv.process(function()
    local face = mv.font {size = 24}
    local score = 0
    local label = mv.write {
        text = "Score: 0", x = center, y = 32, font = face,
        align = mv.TEXT_ALIGN_CENTER,
    }
    while not mv.key(mv.KEY_ESCAPE) do
        if mv.key_pressed(mv.KEY_SPACE) then
            score = score + 1
            mv.change_text(label, {text = "Score: " .. score})
        end
        mv.draw_text {
            text = "Press Space to score", x = center, y = 92, font = face,
            align = mv.TEXT_ALIGN_CENTER, color = mv.colors.YELLOW,
        }
        mv.frame()
    end
end)

return {title = "Text and fonts", width = 320, height = 160, init = function() scoreboard() end}
