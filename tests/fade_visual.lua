-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local function draw_frames(count)
  for _ = 1, count do
    mv.set_color(255, 255, 255)
    mv.draw_box(0, 0, 64, 64)
    mv.frame()
  end
end
local task = mv.process(function()
  draw_frames(15)
  mv.fade_off(4)
  draw_frames(20)
  mv.fade_on(4)
  draw_frames(15)
end)
mv.start(function() task() end, {width = 64, height = 64, title = "Fade test"})
