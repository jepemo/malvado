-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

require("malvado").import_globals()

local WIDTH = 640
local HEIGHT = 360

-- The player keeps its own position. The engine draws its rectangle.
local Player = process(function(self)
    while not key(KEY_ESCAPE) do
        local distance = 400 * self.delta
        if key(KEY_LEFT) then self.x = self.x - distance end
        if key(KEY_RIGHT) then self.x = self.x + distance end
        self.x = math.max(40, math.min(WIDTH - 40, self.x))
        frame()
    end
end, {defaults = {graph = rect {width = 80, height = 16, color = CYAN}}})

-- The ball falls, tests its own collision with the player, and keeps the score.
local Ball = process(function(self)
    local score = 0
    while not key(KEY_ESCAPE) do
        self.y = self.y + 180 * self.delta

        local caught = collision(Player)
        if caught then score = score + 1 end
        if caught or self.y > HEIGHT + 12 then
            self.x = (self.x + 160) % WIDTH
            self.y = -12
        end

        draw_text {text = "Score: " .. score, x = 20, y = 20, size = 24}
        frame()
    end
end, {defaults = {graph = circle {radius = 12, color = ORANGE}}})

start(function()
    Player({x = WIDTH / 2, y = HEIGHT - 40})
    Ball({x = 80, y = 0})
end, {title = "Catch!", width = WIDTH, height = HEIGHT})
