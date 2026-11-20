-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

require("malvado").import_globals()

local W, H = 800, 600

-- Every ball is one process. It moves by itself and bounces on the screen edges.
local Ball = process(function(self)
    while true do
        self.x = self.x + self.vx
        self.y = self.y + self.vy
        if self.x < 16 or self.x > W - 16 then self.vx = -self.vx end
        if self.y < 16 or self.y > H - 16 then self.vy = -self.vy end
        frame()
    end
end, {defaults = {graph = circle(16, {color = {80, 190, 255}})}})

-- The player follows the arrow keys. Hold Space to move faster.
local Player = process(function(self)
    while true do
        local speed = key(KEY_SPACE) and 8 or 4
        if key(KEY_LEFT) then self.x = self.x - speed end
        if key(KEY_RIGHT) then self.x = self.x + speed end
        if key(KEY_UP) then self.y = self.y - speed end
        if key(KEY_DOWN) then self.y = self.y + speed end
        frame()
    end
end, {defaults = {graph = circle(24, {color = {255, 190, 80}})}})

-- The screen process starts the balls and the player. They are its children:
-- when the screen ends on Escape, they end with it and the game closes.
local Screen = process(function()
    Ball({x = 120, y = 140, vx = 3, vy = 2})
    Ball({x = 640, y = 420, vx = -4, vy = 3})
    Ball({x = 400, y = 500, vx = 2, vy = -4})
    Player({x = 400, y = 300})
    while not key(KEY_ESCAPE) do
        frame()
    end
end)

start(Screen, {title = "Move", width = W, height = H})
