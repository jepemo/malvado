-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
mv.import_globals()
local red = mv.graphics.rect(40, 30, {pivot = "top-left", color = {255, 0, 0}})
local green = mv.graphics.rect(8, 8, {pivot = "top-left", color = {0, 255, 0}})
local blue_region = mv.graphics.region(-3, 40, 8, 8)
local square = process(function(self)
  frame()
  self.region = mv.graphics.region(25, 10, 10, 20)
  while true do frame() end
end, {defaults = {graph = red, region = mv.graphics.region(20, 10, 10, 20)}})
local free = process(function() while true do frame() end end, {defaults = {graph = green}})
local blue = process(function()
  while true do
    set_color(0, 0, 255)
    draw_box(0, 35, 10, 50)
    frame()
  end
end, {defaults = {region = blue_region}})
local hidden = process(function() while true do frame() end end,
  {defaults = {graph = red, region = mv.graphics.region(100, 100, 5, 5)}})
local game = {width = 64, height = 64, title = "Regions test"}
function game.init()
  clear_screen(0, 0, 0)
  square({x = 10, y = 10})
  free({x = 42, y = 10, z = 1})
  blue({z = 2})
  hidden({x = 0, y = 0, z = 3})
end
return game
