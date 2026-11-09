-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

require("malvado").import_globals()

local atlas
local Player = process(function(self)
  self.file = atlas
  self.x, self.y, self.size = 280, 190, 16
  local elapsed = 0
  while not key("escape") do
    local sprite = animation_frame(atlas, "bounce", math.floor(elapsed))
    self.graph = graph(sprite)
    draw_text("Aseprite animation  |  Escape: exit", 20, 20, 20)
    elapsed = elapsed + 1000 / 60
    frame()
  end
end)

local game = {width = 640, height = 480, title = "Aseprite animation"}
function game.init()
  clear_screen(25, 30, 45)
  atlas = load_asset("examples/aseprite/sheet.json")
  Player()
end
return game
