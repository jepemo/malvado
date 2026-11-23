-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
mv.import_globals()

local Scene = process(function(self)
  local scroll_x, scroll_y = 80, 40
  while not key("escape") do
    if key("left") then scroll_x = scroll_x - 3 end
    if key("right") then scroll_x = scroll_x + 3 end
    if key("up") then scroll_y = scroll_y - 3 end
    if key("down") then scroll_y = scroll_y + 3 end
    self.scroll = scroll(scroll_x, scroll_y)
    draw_rect {x = 280, y = 180, width = 60, height = 60, color = {255, 165, 70}}
    frame()
  end
end, {defaults = {
  graph = rect {width = 80, height = 60, pivot = "top-left", color = {80, 190, 255}},
  region = region {x = 40, y = 60, width = 300, height = 240}
}})

local Overlay = process(function()
  while not key("escape") do
    draw_rect {x = 40, y = 60, width = 300, height = 240, color = {240, 240, 240}, filled = false}
    draw_text {text = "Arrow keys: scroll  |  Escape: exit", x = 20, y = 20, size = 20}
    frame()
  end
end)

local game = {width = 640, height = 480, title = "Process scrolling"}
function game.init()
  clear_screen {25, 30, 45, 255}
  Scene({x = 140, y = 130})
  Overlay({z = 1})
end
return game
