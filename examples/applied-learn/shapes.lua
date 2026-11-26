-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")

local show = mv.process(function(self)
    self.graph = mv.circle {radius = 16, color = mv.colors.ORANGE}
    self.x, self.y = 180, 72

    while not mv.key(mv.KEY_ESCAPE) do
        mv.draw_rect {x = 24, y = 24, width = 72, height = 48,
                      color = {70, 170, 255}, filled = false}
        mv.draw_line {x1 = 24, y1 = 72, x2 = 180, y2 = 72, color = mv.colors.WHITE}
        mv.frame()
    end
end)

return {title = "Drawing and coordinates", width = 260, height = 140, init = function() show() end}
