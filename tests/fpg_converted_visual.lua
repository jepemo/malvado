-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
mv.import_globals()
local red, green
local pivot_x = assert(tonumber(os.getenv("MALVADO_TEST_PIVOT_X")))
local pivot_y = assert(tonumber(os.getenv("MALVADO_TEST_PIVOT_Y")))
local directory = assert(os.getenv("MALVADO_TEST_FPG_DIR"))
local Picture = process(function()
  while true do
    draw_image_region(red, 160, 120, 0, 0, 2, 2, pivot_x, pivot_y, 32)
    draw_image_region(green, 240, 120, 0, 0, 2, 1, 0, 0, 32)
    frame()
  end
end)
local game = {width = 640, height = 480, title = "Converted FPG"}
function game.init()
  clear_screen(25, 30, 45)
  red = assert(mv.load_image(directory .. "/7.png"))
  green = assert(mv.load_image(directory .. "/42.png"))
  Picture()
end
return game
