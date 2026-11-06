-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
mv.import_globals()
local box = mv.graphics.rect(20, 20, {pivot = "top-left", color = {255, 0, 0}})
local square = process(function(self)
  frame()
  self.scroll = mv.graphics.scroll(10, 5)
  while true do frame() end
end, {defaults = {graph = box, region = mv.graphics.region(10, 10, 20, 20),
                   scroll = mv.graphics.scroll(0, 0)}})
local hud = process(function()
  while true do
    set_color(0, 255, 0)
    draw_box(45, 10, 50, 15)
    frame()
  end
end)
local blue = process(function()
  while true do
    set_color(0, 0, 255)
    draw_box(10, 40, 20, 50)
    frame()
  end
end, {defaults = {scroll = mv.graphics.scroll(5, 0),
                   region = mv.graphics.region(0, 40, 10, 10)}})
local game = {width = 64, height = 64, title = "Scroll test"}
function game.init()
  clear_screen(0, 0, 0)
  square({x = 20, y = 20})
  hud({z = 1})
  blue({z = 2})
end
return game
