-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
assert(not pcall(mv.draw_box, 0, 0, 5, 5))
assert(not pcall(mv.set_color, {-1, 0, 0, 255}))
local ok, message = pcall(mv.set_color, {-1, 0, 0, 255})
assert(not ok and message:find("color channel 1 must be an integer in 0..255", 1, true))
assert(not pcall(mv.set_color, {0, 0, 256, 255}))
mv.clear_screen {0, 127, 255, 255}
for channel = 1, 3 do
  for _, value in ipairs({-1, 256, 0x100000000, -0x100000000, math.maxinteger, math.mininteger}) do
    local rgba = {12, 34, 56, 255}
    rgba[channel] = value
    assert(not pcall(mv.clear_screen, rgba))
  end
end
local task = mv.process(function()
  for _, key in ipairs({"x", "y", "size"}) do
    for _, value in ipairs({0x100000000, -0x100000000, math.maxinteger, math.mininteger}) do
      local options = {text = "text", x = 0, y = 0, size = 20}
      options[key] = value
      assert(not pcall(mv.draw_text, options))
    end
  end
  assert(not pcall(mv.draw_text, {text = "A\0B", x = 0, y = 0, size = 20}))
  assert(mv.snapshot().processes[1].draw_commands == 0)
  mv.draw_text {text = "text", x = -2147483648, y = 2147483647, size = 20}
  assert(not pcall(mv.draw_box, 5, 5, 4, 6))
  assert(not pcall(mv.draw_rect, 0, 0, 2^40, 1))
  assert(not pcall(mv.draw_circle, 0, 0, -1))
  assert(not pcall(mv.draw_fcircle, 2147483647, 0, 1))
  mv.set_color {10, 20, 30, 40}
  mv.draw_box(0, 0, 5, 5)
  mv.draw_rect(0, 0, 5, 5)
  mv.draw_circle(5, 5, 2)
  mv.draw_fcircle(5, 5, 2)
  local snapshot = mv.snapshot()
  assert(snapshot.processes[1].draw_commands == 5)
  mv.frame()
end)
mv.start(function() task() end)
