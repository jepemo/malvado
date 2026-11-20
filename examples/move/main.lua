-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

require("malvado").import_globals()

-- One process draws a circle and moves it with the arrow keys.
local Circle = process(function(self)
    while not key(KEY_ESCAPE) do
        if key(KEY_LEFT) then self.x = self.x - 4 end
        if key(KEY_RIGHT) then self.x = self.x + 4 end
        if key(KEY_UP) then self.y = self.y - 4 end
        if key(KEY_DOWN) then self.y = self.y + 4 end
        frame()
    end
end, {defaults = {graph = circle(24, {color = {255, 190, 80}})}})

start(function() Circle({x = 400, y = 300}) end, {title = "Move"})
