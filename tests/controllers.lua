-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
mv.import_globals()
local function constant(prefix, name)
  local key = prefix .. name:upper()
  assert(mv[key] == name and mv.input[key] == name and _G[key] == name)
  return mv[key]
end
for slot = 0, 3 do
  assert(not mv.pad_connected(slot))
  for _, button in ipairs({"a", "b", "x", "y", "up", "down", "left", "right", "lb", "rb", "back", "start"}) do
    assert(not mv.pad_button(slot, constant("PAD_", button)))
    assert(not mv.pad_pressed(slot, button) and not mv.pad_released(slot, button))
  end
  for _, axis in ipairs({"left_x", "left_y", "right_x", "right_y", "left_trigger", "right_trigger"}) do
    assert(mv.pad_axis(slot, constant("PAD_", axis)) == 0)
  end
end
assert(not pcall(mv.pad_connected, -1))
assert(not pcall(mv.pad_connected, 4))
assert(not pcall(mv.pad_connected, 0.5))
assert(not pcall(mv.pad_button, 0, "A"))
assert(not pcall(mv.pad_axis, 0, "left_x\0extra"))
assert(not pcall(mv.pad_button, 4, "a"))
assert(not pcall(mv.pad_button, 0, "a\0b"))
assert(not pcall(mv.pad_axis, 0, "bad"))

for _, query in ipairs({mv.pad_button, mv.pad_pressed, mv.pad_released}) do
  for _, slot in ipairs({-1, 4, 0.5}) do
    assert(not pcall(query, slot, PAD_A))
  end
  assert(not pcall(query, PAD_A))
  for _, name in ipairs({"", "A", "a ", "a\0extra"}) do
    assert(not pcall(query, 0, name))
  end
end
for _, name in ipairs({"", "LEFT_X", "left_x ", "left_x\0extra"}) do
  assert(not pcall(mv.pad_axis, 0, name))
end
