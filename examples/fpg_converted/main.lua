-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
mv.import_globals()

local red, green, pivot
local Picture = process(function()
  while not key("escape") do
    draw_image_region(red, 160, 120, 0, 0, 2, 2, pivot.x, pivot.y, 32)
    draw_image_region(green, 240, 120, 0, 0, 2, 1, 0, 0, 32)
    draw_text("Converted FPG  |  Escape: exit", 20, 20, 20)
    frame()
  end
end)

local game = {width = 640, height = 480, title = "Converted FPG"}
function game.init()
  clear_screen(25, 30, 45)
  local file = assert(io.open("assets/manifest.json", "rb"))
  local manifest = mv.fpg.parse_manifest(file:read("a"))
  file:close()
  red = assert(mv.load_image("assets/" .. manifest:sprite(7).image))
  green = assert(mv.load_image("assets/" .. manifest:sprite(42).image))
  pivot = assert(manifest:point(7, 0))
  Picture()
end
return game
