-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

require("malvado").import_globals()

local Picture = process(function(self)
  self.graph = graph(self.sprite)
  while not key("escape") do
    if self.help then
      draw_text("Converted FPG  |  Escape: exit", 20, 20, 20)
    end
    frame()
  end
end)

local game = {width = 640, height = 480, title = "Converted FPG"}
function game.init()
  clear_screen(25, 30, 45)
  local library = load_asset("assets/manifest.json")
  Picture({file = library, sprite = 7, x = 160, y = 120, size = 32, help = true})
  Picture({file = library, sprite = 42, x = 240, y = 120, size = 32})
end
return game
