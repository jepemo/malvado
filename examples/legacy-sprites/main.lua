-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
require("malvado.compat.legacy").import_globals()
local library
local smoke = os.getenv("MALVADO_SMOKE") == "1"

local Cat = process(function(self)
  self.fpg = library
  self.fpgIndex = 0
  self.fps = 15
  local ticks = 0
  while not key("escape") do
    ticks = ticks + 1
    self.fpgIndex = (self.fpgIndex + 1) % 6
    if smoke and ticks == 4 then
      print("legacy sprites smoke: animated directory")
      return
    end
    frame()
  end
end)

mv.start(function()
  library = fpg("examples/demo/assets/cat")
  clear_screen(25, 30, 45)
  Cat {x = 400, y = 300}
end, {width = 800, height = 600, title = "Legacy sprites"})
