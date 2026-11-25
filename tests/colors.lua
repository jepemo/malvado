-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local palette = {
  BLACK = {0, 0, 0, 255},
  WHITE = {255, 255, 255, 255},
  TRANSPARENT = {0, 0, 0, 0},
  GRAY = {128, 128, 128, 255},
  LIGHT_GRAY = {211, 211, 211, 255},
  DARK_GRAY = {64, 64, 64, 255},
  RED = {255, 0, 0, 255},
  GREEN = {0, 255, 0, 255},
  BLUE = {0, 0, 255, 255},
  YELLOW = {255, 255, 0, 255},
  CYAN = {0, 255, 255, 255},
  MAGENTA = {255, 0, 255, 255},
  ORANGE = {255, 165, 0, 255},
  PURPLE = {128, 0, 128, 255},
  PINK = {255, 192, 203, 255},
  BROWN = {165, 42, 42, 255}
}
local keys = {"r", "g", "b", "a"}

for name, expected in pairs(palette) do
  assert(rawget(_G, name) == nil and mv[name] == nil)
  local color = assert(mv.colors[name])
  assert(type(color) == "userdata")
  for i, key in ipairs(keys) do
    assert(color[i] == expected[i] and color[key] == expected[i], name .. "." .. key)
  end
  assert(not pcall(function() color[4] = -1 end))
  assert(not pcall(function() color.a = -1 end))
  assert(not pcall(rawset, color, 4, -1))
  assert(not pcall(setmetatable, color, {}))
  assert(color[4] == expected[4] and color.a == expected[4])
end
for name in pairs(mv.colors) do assert(palette[name], name) end
assert(mv.colors.RED[0] == nil and mv.colors.RED["r\0"] == nil)

RED = {255, 0, 0, 255}
local ok, message = pcall(mv.import_globals)
assert(not ok and message:find("global already exists: RED", 1, true))
assert(set_color == nil and BLACK == nil)
RED = nil
mv.import_globals()
mv.import_globals()
for name in pairs(palette) do assert(rawget(_G, name) == mv.colors[name]) end
assert(colors == nil and COLOR_RED == nil)
RED = "rebound"
assert(mv.colors.RED.r == 255)
RED = mv.colors.RED

local function assert_rgba(color, expected)
  for i = 1, 4 do assert(color[i] == expected[i], "channel " .. i) end
end

local function check_fade(color, expected)
  mv.fade_to {color = color, duration_ms = 0}
  assert_rgba(mv.fade_color(), expected)
end

local task = mv.process(function()
  local font = mv.font {size = 20}
  local id = mv.write {font = font, text = "X", x = 0, y = 0, color = WHITE}
  local function apply_color(color)
    mv.set_color(color)
    mv.clear_screen(color)
    mv.draw_text {text = "X", x = 10, y = 10, color = color}
    mv.change_text(id, {color = color})
    mv.delete_text(mv.write {font = font, text = "X", x = 0, y = 0, color = color})
    assert(mv.rect {width = 2, height = 2, color = color})
    assert(mv.graphics.circle {radius = 2, color = color})
  end
  for _, color in ipairs({
    {255, 0, 0}, {255, 0, 0, 255}, {r = 255, g = 0, b = 0},
    {r = 255, g = 0, b = 0, a = 255}, RED
  }) do
    apply_color(color)
    check_fade(color, palette.RED)
  end
  local positional = {1, 2, 3}
  positional[4] = 80
  local named = {r = 1, g = 2, b = 3}
  named.a = 80
  for _, color in ipairs({positional, named}) do
    apply_color(color)
    check_fade(color, {1, 2, 3, 80})
  end
  for name, expected in pairs(palette) do check_fade(mv.colors[name], expected) end

  check_fade(TRANSPARENT, palette.TRANSPARENT)
  for _, invalid in ipairs({
    false, "red", 0, {}, {255, 0}, {1, 2, nil, 4}, {r = 255, g = 0},
    {1, 2, 3, 4, 5}, {1, 2, 3, extra = 4}, {1, 2, 3, [0] = 4},
    {r = 1, g = 2, b = 3, ["a\0"] = 4}, {r = 1, g = 2, b = 3, [4] = 4},
    {1, 2, 3, a = 4}, {1, 2, 3, [true] = 4}, {r = 1, g = 2, b = 3, alpha = 4},
    {1, 2, 256}, {1, -1, 3}, {1, 2, "3"}, {1, 2, 3.0}, {r = 1, g = 2, b = 2.5},
    {1, 2, 3, false}, {1, 2, 3, math.huge}, {1, 2, 3, 0 / 0},
    setmetatable({1, 2}, {__index = function() return 3 end})
  }) do
    assert(not pcall(mv.set_color, invalid))
    assert(not pcall(mv.clear_screen, invalid))
    assert(not pcall(mv.draw_text, {text = "X", x = 0, y = 0, color = invalid}))
    assert(not pcall(mv.write, {font = font, text = "X", x = 0, y = 0, color = invalid}))
    assert(not pcall(mv.change_text, id, {color = invalid}))
    assert(not pcall(mv.rect, 2, 2, {color = invalid}))
    assert(not pcall(mv.graphics.circle, 2, {color = invalid}))
    assert(not pcall(mv.fade_to, {color = invalid, duration_ms = 0}))
  end
  assert_rgba(mv.fade_color(), palette.TRANSPARENT)
end)
mv.start(function() task() end)
