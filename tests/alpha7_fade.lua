-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local task = mv.process(function()
  mv.fade_to {color = {0, 0, 0, 255}, duration_ms = 30}
  mv.fade_to {color = {20, 40, 60, 80}, duration_ms = 0}
  mv.fade_to {color = {0, 20, 40, 0}, duration_ms = 30}
  mv.fade_to {color = {1, 2, 3, 4}, duration_ms = 0}
  for tick = 0, 4 do
    local color = mv.fade_color()
    print(string.format("%d %d %d %d %d %d", tick, color[1], color[2], color[3], color[4],
      mv.fade_active() and 1 or 0))
    if tick < 4 then mv.frame() end
  end
end)
mv.start(function() task() end)
