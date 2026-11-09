-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
assert(not pcall(mv.draw_box, 0, 0, 5, 5))
assert(not pcall(mv.set_color, -1, 0, 0))
assert(not pcall(mv.set_color, 0, 0, 256))
mv.clear_screen(0, 127, 255)
for channel = 1, 3 do
  for _, value in ipairs({-1, 256, 0x100000000, -0x100000000, math.maxinteger, math.mininteger}) do
    local rgb = {12, 34, 56}
    rgb[channel] = value
    assert(not pcall(mv.clear_screen, table.unpack(rgb)))
  end
end
local task = mv.process(function()
  assert(not pcall(mv.draw_box, 5, 5, 4, 6))
  assert(not pcall(mv.draw_rect, 0, 0, 2^40, 1))
  assert(not pcall(mv.draw_circle, 0, 0, -1))
  assert(not pcall(mv.draw_fcircle, 2147483647, 0, 1))
  mv.set_color(10, 20, 30, 40)
  mv.draw_box(0, 0, 5, 5)
  mv.draw_rect(0, 0, 5, 5)
  mv.draw_circle(5, 5, 2)
  mv.draw_fcircle(5, 5, 2)
  local snapshot = mv.snapshot()
  assert(snapshot.processes[1].draw_commands == 4)
  mv.frame()
end)
mv.start(function() task() end)
