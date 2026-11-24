-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

require("malvado").import_globals()

-- One process draws a circle and moves it with the arrow keys.
local Player = process(function(self)
    local speed = 240 -- Pixels per second.
    while not key(KEY_ESCAPE) do
        local distance = speed * self.delta
        if key(KEY_LEFT) then self.x = self.x - distance end
        if key(KEY_RIGHT) then self.x = self.x + distance end
        if key(KEY_UP) then self.y = self.y - distance end
        if key(KEY_DOWN) then self.y = self.y + distance end
        frame()
    end
end, {defaults = {graph = circle {radius = 24, color = ORANGE}}})

start(function() Player({x = 400, y = 300}) end, {title = "Movement"})
