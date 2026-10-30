-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local task = mv.process(function()
  assert(not mv.fade_active())
  assert(not pcall(mv.fade_to, 0, 0, 0, 256, 2))
  mv.fade_off(2)
  mv.fade_on(2)
  local expected = {0, 127, 255, 128, 0}
  for i = 1, #expected do
    local _, _, _, alpha = mv.fade_color()
    assert(alpha == expected[i], string.format("step %d: %d", i, alpha))
    assert(mv.fade_active() == (i < #expected))
    if i < #expected then mv.frame() end
  end
  mv.fade_to(10, 20, 30, 40, 0)
  local r, g, b, a = mv.fade_color()
  assert(r == 10 and g == 20 and b == 30 and a == 40)
  mv.fade_off(1)
  for _ = 1, 16 do mv.fade_on(0) end
  assert(not pcall(mv.fade_on, 0))
end)
mv.start(function() task() end)
