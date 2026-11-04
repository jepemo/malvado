-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
mv.import_globals()

local Square = process(function()
  while not key("escape") do frame() end
end, {defaults = {graph = mv.graphics.rect(100, 100, {
  pivot = "top-left", color = {255, 165, 70}
})}})

local Circle = process(function(self)
  while not key("escape") do
    if key("left") then self.x = self.x - 4 end
    if key("right") then self.x = self.x + 4 end
    if key("up") then self.y = self.y - 4 end
    if key("down") then self.y = self.y + 4 end
    self.x = math.max(20, math.min(620, self.x))
    self.y = math.max(20, math.min(460, self.y))
    draw_text("Arrow keys: move  |  Escape: exit", 20, 20, 20)
    if process.collision(self, Square) then
      draw_text("Collision!", 20, 55, 28)
    end
    frame()
  end
end, {defaults = {graph = mv.graphics.circle(20, {color = {80, 190, 255}})}})

local game = {width = 640, height = 480, title = "Circle collision"}
function game.init()
  clear_screen(25, 30, 45)
  Square({x = 400, y = 180})
  Circle({x = 120, y = 230})
end
return game
