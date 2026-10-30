-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local start = require("malvado").start
require("malvado.compat.legacy").import_globals()
local library

local Cat = process(function(self)
  self.fpg = library
  self.fpgIndex = 0
  self.fps = 15
  while not key("escape") do
    self.fpgIndex = (self.fpgIndex + 1) % 6
    frame()
  end
end)

start(function()
  library = fpg("examples/demo/assets/cat")
  clear_screen(25, 30, 45)
  Cat {x = 400, y = 300}
end, {width = 800, height = 600, title = "Legacy sprites"})
