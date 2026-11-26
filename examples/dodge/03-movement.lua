-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

require("malvado").import_globals()

local WIDTH, HEIGHT = 640, 360
local PLAYER_WIDTH = 64

local Player = process(function(self)
    while not key(KEY_ESCAPE) do
        local distance = 300 * self.delta
        if key(KEY_LEFT) then self.x = self.x - distance end
        if key(KEY_RIGHT) then self.x = self.x + distance end
        local half_width = PLAYER_WIDTH / 2
        self.x = math.max(half_width, math.min(WIDTH - half_width, self.x))
        frame()
    end
end, {defaults = {graph = rect {width = PLAYER_WIDTH, height = 16, color = CYAN}}})

return {
    title = "Dodge!",
    width = WIDTH,
    height = HEIGHT,
    init = function()
        Player {x = WIDTH / 2, y = HEIGHT - 40}
    end,
}
