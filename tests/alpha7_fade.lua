-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local task = mv.process(function()
  mv.fade_to(0, 0, 0, 255, 2)
  mv.fade_to(20, 40, 60, 80, 0)
  mv.fade_to(0, 20, 40, 0, 2)
  mv.fade_to(1, 2, 3, 4, 0)
  for tick = 0, 4 do
    local r, g, b, a = mv.fade_color()
    print(string.format("%d %d %d %d %d %d", tick, r, g, b, a, mv.fade_active() and 1 or 0))
    if tick < 4 then mv.frame() end
  end
end)
mv.start(function() task() end)
