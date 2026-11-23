-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
mv.import_globals()
local box = mv.graphics.rect {width = 20, height = 20, pivot = "top-left", color = {255, 0, 0}}
local square = process(function(self)
  frame()
  self.scroll = mv.graphics.scroll(10, 5)
  while true do frame() end
end, {defaults = {graph = box, region = mv.graphics.region {x = 10, y = 10, width = 20, height = 20},
                   scroll = mv.graphics.scroll(0, 0)}})
local hud = process(function()
  while true do
    set_color {0, 255, 0, 255}
    draw_rect {x = 45, y = 10, width = 5, height = 5}
    frame()
  end
end)
local blue = process(function()
  while true do
    set_color {0, 0, 255, 255}
    draw_rect {x = 10, y = 40, width = 10, height = 10}
    frame()
  end
end, {defaults = {scroll = mv.graphics.scroll(5, 0),
                   region = mv.graphics.region {x = 0, y = 40, width = 10, height = 10}}})
local game = {width = 64, height = 64, title = "Scroll test"}
function game.init()
  clear_screen {0, 0, 0, 255}
  square({x = 20, y = 20})
  hud({z = 1})
  blue({z = 2})
end
return game
