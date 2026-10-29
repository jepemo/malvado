-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local Hello = mv.process(function(self)
  local font = mv.font(40, 255, 255, 255)
  local greeting = mv.write(font, self.x, self.y, "Hello, Malvado")
  local smoke = os.getenv("MALVADO_SMOKE") == "1"
  local ticks = 0
  while not mv.key("escape") do
    ticks = ticks + 1
    if smoke and ticks >= 3 then break end
    mv.frame()
  end
  mv.delete_text(greeting)
end)
mv.start(function()
  mv.clear_screen(25, 30, 45)
  Hello { x = 60, y = 80 }
end, { width = 800, height = 600, title = "Malvado alpha" })
