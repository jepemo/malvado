-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

require("malvado").import_globals()

local WIDTH, HEIGHT = 640, 360
local PLAYER_WIDTH = 64
local ENEMY_RADIUS = 12

local Player = process(function(self)
    while true do
        local distance = 300 * self.delta
        if key(KEY_LEFT) then self.x = self.x - distance end
        if key(KEY_RIGHT) then self.x = self.x + distance end
        local half_width = PLAYER_WIDTH / 2
        self.x = math.max(half_width, math.min(WIDTH - half_width, self.x))
        frame()
    end
end, {defaults = {graph = rect {width = PLAYER_WIDTH, height = 16, color = CYAN}}})

local Enemy = process(function(self)
    while self.y < HEIGHT + ENEMY_RADIUS do
        self.y = self.y + 140 * self.delta
        frame()
    end
end, {defaults = {graph = circle {radius = ENEMY_RADIUS, color = ORANGE}}})

local Round = process(function(self)
    local player = Player {x = WIDTH / 2, y = HEIGHT - 40}
    local lanes = {WIDTH / 2, 120, 520, 220, 420}
    local lane = 1
    local until_spawn = 0
    local survived = 0
    while not key(KEY_ESCAPE) do
        if player:collision(Enemy) then
            return
        end
        survived = survived + self.delta
        draw_text {text = ("Time: %.1f s"):format(survived), x = 20, y = 20, size = 24}
        until_spawn = until_spawn - self.delta
        if until_spawn <= 0 then
            Enemy {x = lanes[lane], y = -ENEMY_RADIUS}
            lane = lane % #lanes + 1
            until_spawn = until_spawn + 0.7
        end
        frame()
    end
end)

return {
    title = "Dodge!",
    width = WIDTH,
    height = HEIGHT,
    init = function()
        Round()
    end,
}
