-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
mv.import_globals()

local Square = process(function()
  while not key(KEY_ESCAPE) do frame() end
end, {defaults = {graph = rect {width = 100, height = 100, pivot = "top-left", color = {255, 165, 70}}}})

local Circle = process(function(self)
  while not key(KEY_ESCAPE) do
    if key(KEY_LEFT) then self.x = self.x - 4 end
    if key(KEY_RIGHT) then self.x = self.x + 4 end
    if key(KEY_UP) then self.y = self.y - 4 end
    if key(KEY_DOWN) then self.y = self.y + 4 end
    self.x = math.max(20, math.min(620, self.x))
    self.y = math.max(20, math.min(460, self.y))
    draw_text {text = "Arrow keys: move  |  Escape: exit", x = 20, y = 20, size = 20}
    if collision(Square) then
      draw_text {text = "Collision!", x = 20, y = 55, size = 28}
    end
    frame()
  end
end, {defaults = {graph = circle {radius = 20, color = {80, 190, 255}}}})

local game = {width = 640, height = 480, title = "Circle collision"}
function game.init()
  clear_screen {25, 30, 45, 255}
  Square({x = 400, y = 180})
  Circle({x = 120, y = 230})
end
return game
