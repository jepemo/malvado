-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
assert(mv.draw_box == nil and mv.draw_fcircle == nil)
assert(not pcall(mv.draw_rect, {x = 0, y = 0, width = 5, height = 5}))
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
  local min, max = -2147483648, 2147483647
  local function rejects(draw, valid, field, values)
    for _, value in ipairs(values) do
      local options = {}
      for name, original in pairs(valid) do options[name] = original end
      if value == false and valid[field] ~= nil then
        options[field] = nil
      else
        options[field] = value
      end
      assert(not pcall(draw, options), field .. " accepted " .. tostring(value))
    end
  end

  local rect = {x = 0, y = 0, width = 5, height = 5}
  local circle = {x = 5, y = 5, radius = 2}
  local line = {x1 = 0, y1 = 0, x2 = 2, y2 = 3}
  for _, item in ipairs({
    {mv.draw_rect, rect}, {mv.draw_circle, circle}, {mv.draw_line, line}
  }) do
    local draw, options = item[1], item[2]
    for name in pairs(options) do
      rejects(draw, options, name, {false, "2", 1.5, min - 1, max + 1, math.huge, 0/0})
    end
    rejects(draw, options, "color", {{256, 0, 0}, "red"})
    rejects(draw, options, "extra", {1})
    assert(not pcall(draw, options, {}))
  end
  rejects(mv.draw_rect, rect, "filled", {1, "true"})
  rejects(mv.draw_circle, circle, "filled", {0, "false"})
  rejects(mv.draw_rect, rect, "width", {0, -1})
  rejects(mv.draw_rect, rect, "height", {0, -1})
  rejects(mv.draw_rect, rect, "x", {max - 4, max})
  rejects(mv.draw_rect, rect, "y", {max - 4, max})
  rejects(mv.draw_circle, circle, "radius", {0, -1, max})
  for _, axis in ipairs({"x", "y"}) do
    rejects(mv.draw_circle, circle, axis, {min, min + 1, max - 1, max})
  end
  rejects(mv.draw_line, line, "thickness", {false, 0, -1, "2", 0/0, math.huge, 1e-40, 1e38})
  assert(mv.snapshot().processes[1].draw_commands == 1)

  mv.set_color {10, 20, 30, 40}
  mv.draw_rect {x = min, y = max - 1, width = 1, height = 1}
  mv.draw_rect {x = 0, y = 0, width = 5, height = 5, filled = false, color = mv.colors.RED}
  mv.draw_circle {x = min + 2, y = max - 2, radius = 2, color = {1, 2, 3}}
  mv.draw_circle {x = 5, y = 5, radius = 2, filled = false, color = {r = 1, g = 2, b = 3, a = 4}}
  mv.draw_line {x1 = min, y1 = 0, x2 = max, y2 = 0}
  mv.draw_line {x1 = 2.0, y1 = 2, x2 = 2, y2 = 3, thickness = 1.5, color = {1, 2, 3, 4}}
  assert(mv.snapshot().processes[1].draw_commands == 7)
  mv.frame()
end)
mv.start(function() task() end)
