-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

require("malvado").import_globals()

local atlas
local Player = process(function()
  local elapsed = 0
  while not key("escape") do
    local sprite = animation_frame(atlas, "bounce", math.floor(elapsed))
    draw_sprite(atlas, sprite, 280, 190, 16)
    draw_text("Aseprite animation  |  Escape: exit", 20, 20, 20)
    elapsed = elapsed + 1000 / 60
    frame()
  end
end)

local game = {width = 640, height = 480, title = "Aseprite animation"}
function game.init()
  clear_screen(25, 30, 45)
  local image = load_image("examples/aseprite/sheet.png")
  local file = assert(io.open("examples/aseprite/sheet.json", "rb"))
  atlas = load_aseprite(image, file:read("a"))
  file:close()
  Player()
end
return game
