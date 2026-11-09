-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
mv.import_globals()

local viewport = region(100, 100, 160, 160)
local Player = process(function(self)
  while not key("escape") do
    if key("left") then self.x = self.x - 3 end
    if key("right") then self.x = self.x + 3 end
    if key("up") then self.y = self.y - 3 end
    if key("down") then self.y = self.y + 3 end
    frame()
  end
end, {defaults = {region = viewport, graph = rect(60, 60, {
  color = {80, 190, 255}
})}})

local Overlay = process(function()
  while not key("escape") do
    set_color(240, 240, 240)
    draw_rect(100, 100, 260, 260)
    draw_text("Arrow keys: move  |  Escape: exit", 20, 20, 20)
    frame()
  end
end)

local game = {width = 640, height = 480, title = "Process region"}
function game.init()
  clear_screen(25, 30, 45)
  Player({x = 80, y = 160})
  Overlay({z = 1})
end
return game
