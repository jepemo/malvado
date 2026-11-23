-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
mv.import_globals()
local red = rect(30, 10, {pivot = "top-left", color = RED})
local blue = circle(10, {color = {r = 0, g = 0, b = 255}})
local green = circle(5, {color = {0, 255, 0}})
local translucent = rect(20, 10, {color = {r = 255, g = 255, b = 255, a = 128}})
local shape = mv.process(function(self)
  while true do
    if self.check_colors then
      set_color(RED)
      draw_box(54, 94, 60, 100)
      set_color {r = 255, g = 0, b = 0}
      draw_box(64, 94, 70, 100)
    end
    mv.frame()
  end
end)
mv.start(function()
  mv.clear_screen(TRANSPARENT)
  -- Failure must leave the black background unchanged.
  assert(not pcall(mv.clear_screen, {255, 256, 255, 255}))
  assert(not pcall(mv.clear_screen, {0x100000000, 128, 128, 255}))
  shape({graph = red, x = 20, y = 20, angle = 90})
  shape({graph = blue, x = 80, y = 40, size = 2})
  shape({graph = green, x = 80, y = 40, z = 1})
  shape({graph = translucent, x = 130, y = 70})
  shape({graph = rect(20, 10), x = 40, y = 90, check_colors = true})
end, {width = 160, height = 120})
