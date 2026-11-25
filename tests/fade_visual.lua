-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local function draw_frames(count, fade_alpha)
  for i = 1, count do
    if fade_alpha and i <= #fade_alpha then
      assert(mv.fade_color()[4] == fade_alpha[i])
      assert(mv.fade_active() == (i < #fade_alpha))
    end
    mv.set_color {255, 255, 255, 255}
    -- Invalid updates must not partially replace the white command color.
    assert(not pcall(mv.set_color, {0, 0, 256, 255}))
    mv.draw_rect {x = 0, y = 0, width = 64, height = 64}
    mv.frame()
  end
end
local task = mv.process(function()
  draw_frames(15)
  mv.fade_off(60)
  draw_frames(20, {0, 63, 127, 191, 255})
  mv.fade_on(60)
  draw_frames(15, {255, 192, 128, 64, 0})
end)
mv.start(function() task() end, {width = 64, height = 64, title = "Fade test"})
