-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

require("malvado").import_globals()

local atlas
local Player = process(function(self)
  self.file = atlas
  self.x, self.y, self.size = 280, 190, 16
  self.graph = animation("bounce")
  while not key("escape") do
    draw_text {text = "Aseprite animation  |  Escape: exit", x = 20, y = 20, size = 20}
    frame()
  end
end)

local game = {width = 640, height = 480, title = "Aseprite animation"}
function game.init()
  clear_screen {25, 30, 45, 255}
  atlas = load_asset("sheet.json")
  Player()
end
return game
