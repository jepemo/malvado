-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local rect = mv.graphics.rect(30, 10, {pivot = "top-left", color = {255, 0, 0}})
local blue = mv.graphics.circle(10, {color = {0, 0, 255}})
local green = mv.graphics.circle(5, {color = {0, 255, 0}})
local translucent = mv.graphics.rect(20, 10, {color = {255, 255, 255, 128}})
local shape = mv.process(function() while true do mv.frame() end end)
mv.start(function()
  mv.clear_screen(0, 0, 0)
  assert(not pcall(mv.clear_screen, 0x100000000, 128, 128))
  shape({graph = rect, x = 20, y = 20, angle = 90})
  shape({graph = blue, x = 80, y = 40, size = 2})
  shape({graph = green, x = 80, y = 40, z = 1})
  shape({graph = translucent, x = 130, y = 70})
end, {width = 160, height = 120})
