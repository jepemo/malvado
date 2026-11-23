-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local task = mv.process(function()
  assert(not mv.fade_active())
  assert(not pcall(mv.fade_to, {color = {0, 0, 0, 256}, ticks = 2}))
  assert(not pcall(mv.fade_to, {color = {0, 0, 0, 255}, ticks = -1}))
  assert(not pcall(mv.fade_to, {color = {0, 0, 0, 255}, ticks = "soon"}))
  assert(not pcall(mv.fade_to, {color = {0, 0}, ticks = 2}))
  mv.fade_off(2)
  mv.fade_on(2)
  local expected = {0, 127, 255, 128, 0}
  for i = 1, #expected do
    local alpha = mv.fade_color()[4]
    assert(alpha == expected[i], string.format("step %d: %d", i, alpha))
    assert(mv.fade_active() == (i < #expected))
    if i < #expected then mv.frame() end
  end
  mv.fade_to {color = {10, 20, 30, 40}, ticks = 0}
  local color = mv.fade_color()
  assert(color[1] == 10 and color[2] == 20 and color[3] == 30 and color[4] == 40)
  mv.fade_off(1)
  for _ = 1, 16 do mv.fade_on(0) end
  assert(not pcall(mv.fade_on, 0))
end)
mv.start(function() task() end)
