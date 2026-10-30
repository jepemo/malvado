-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local legacy = require("malvado.compat.legacy")
assert(process == nil and frame == nil)
assert(not pcall(mv.window_size))

key = "existing value"
local ok, message = pcall(legacy.import_globals)
assert(not ok and message:find("global already exists: key", 1, true))
assert(process == nil and key == "existing value")
key = nil
legacy.import_globals()
legacy.import_globals()
assert(process == mv.process and frame ~= mv.frame and key == mv.key)
assert(fpg == mv.legacy_fpg)
assert(draw_box == mv.draw_box and set_color == mv.set_color)
assert(not pcall(frame))
assert(set_title == mv.set_title and not pcall(set_title, "Headless"))
math.randomseed(42)
local value = rand(-5, 5)
math.randomseed(42)
assert(value == math.random(-5, 5))
assert(rand(7, 7) == 7)
assert(not pcall(rand, 2, 1))
assert(not pcall(rand, 0.5, 2))

local ran = false
local task = process(function()
  assert(not key("escape"))
  set_color(255, 0, 0)
  draw_box(1, 1, 5, 5)
  frame()
  ran = true
end)
mv.start(function()
  local width, height = mv.window_size()
  assert(width == 321 and height == 234)
  assert(get_screen_width() == width and get_screen_height() == height)
  task()
end, {width = 321, height = 234})
assert(ran)
