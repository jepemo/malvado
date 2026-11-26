-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")

local show = mv.process(function()
    -- Draw the same scene on each activation for two seconds.
    for _ = 1, 120 do
        mv.draw_rect {x = 20, y = 56, width = 64, height = 44}
        mv.draw_rect {x = 96, y = 56, width = 64, height = 44, filled = false}
        mv.draw_circle {x = 202, y = 78, radius = 22, color = mv.colors.ORANGE}
        mv.draw_circle {x = 262, y = 78, radius = 22, color = mv.colors.ORANGE, filled = false}
        mv.draw_line {x1 = 20, y1 = 118, x2 = 284, y2 = 118, thickness = 2}
        mv.draw_text {text = "Filled and outlined shapes", x = 152, y = 14,
                      align = mv.TEXT_ALIGN_CENTER, size = 18}
        mv.frame()
    end
end)

mv.start(function()
    mv.clear_screen {24, 30, 44}
    mv.set_color(mv.colors.BLUE)
    show()
end, {title = "Drawing shapes", width = 304, height = 140})
