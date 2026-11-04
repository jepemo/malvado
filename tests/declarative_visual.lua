-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local task = mv.process(function()
  for _ = 1, 20 do
    mv.set_color(255, 0, 0)
    mv.draw_box(0, 0, 40, 40)
    mv.frame()
  end
end)
return {
  title = "Declarative graphics test",
  width = 320,
  height = 240,
  init = function()
    local width, height = mv.window_size()
    assert(width == 320 and height == 240)
    task()
  end,
}
