-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
for slot = 0, 3 do
  assert(not mv.pad_connected(slot))
  for _, button in ipairs({"a", "b", "x", "y", "up", "down", "left", "right", "lb", "rb", "back", "start"}) do
    assert(not mv.pad_button(slot, button))
  end
  for _, axis in ipairs({"left_x", "left_y", "right_x", "right_y", "left_trigger", "right_trigger"}) do
    assert(mv.pad_axis(slot, axis) == 0)
  end
end
assert(not pcall(mv.pad_connected, -1))
assert(not pcall(mv.pad_button, 4, "a"))
assert(not pcall(mv.pad_button, 0, "a\0b"))
assert(not pcall(mv.pad_axis, 0, "bad"))
