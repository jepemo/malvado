-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local width, height = 320, 180
local radius, speed = 12, 150

local player = mv.process(function(self)
    local target = nil
    while not mv.key(mv.KEY_ESCAPE) do
        local dx, dy = 0, 0
        if mv.key(mv.KEY_LEFT) then dx = dx - 1 end
        if mv.key(mv.KEY_RIGHT) then dx = dx + 1 end
        if mv.key(mv.KEY_UP) then dy = dy - 1 end
        if mv.key(mv.KEY_DOWN) then dy = dy + 1 end

        local length = math.sqrt(dx * dx + dy * dy)
        if length > 0 then
            local distance = speed * self.delta
            self.x = self.x + dx * distance / length
            self.y = self.y + dy * distance / length
        end
        if mv.key_pressed(mv.KEY_SPACE) then
            self.x, self.y = width / 2, height / 2
        end
        self.x = math.max(radius, math.min(width - radius, self.x))
        self.y = math.max(radius, math.min(height - radius, self.y))

        if mv.mouse_pressed(mv.MOUSE_LEFT) then
            local x, y = mv.mouse_position()
            target = {x = x, y = y, radius = 4, color = mv.colors.YELLOW}
        end
        if target then mv.draw_circle(target) end
        mv.frame()
    end
end)

return {
    title = "Input and movement", width = width, height = height,
    init = function()
        player {x = width / 2, y = height / 2,
                graph = mv.circle {radius = radius, color = mv.colors.ORANGE}}
    end,
}
