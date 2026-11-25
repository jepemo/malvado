-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local legacy = require("malvado.compat.legacy")
assert(process == nil and frame == nil)
assert(get_screen_width == nil and get_screen_height == nil)
assert(not pcall(mv.window_size))

get_screen_height = "existing value"
local ok, message = pcall(legacy.import_globals)
assert(not ok and message:find("global already exists: get_screen_height", 1, true))
assert(process == nil and get_screen_width == nil and get_screen_height == "existing value")
get_screen_height = nil
fade_off = mv.fade_off
ok, message = pcall(legacy.import_globals)
assert(not ok and message:find("global already exists: fade_off", 1, true))
assert(process == nil and fade_on == nil and fade_off == mv.fade_off)
fade_off = nil
legacy.import_globals()
local legacy_off, legacy_on = fade_off, fade_on
legacy.import_globals()
assert(process == mv.process and frame ~= mv.frame and key == mv.key)
assert(fpg == mv.legacy_fpg)
assert(draw_rect == mv.draw_rect and draw_circle == mv.draw_circle)
assert(draw_line == mv.draw_line and set_color == mv.set_color)
assert(fade_off == legacy_off and fade_on == legacy_on)
assert(fade_off ~= mv.fade_off and fade_on ~= mv.fade_on)
assert(not pcall(mv.import_globals) and fade_off == legacy_off and fade_active == nil)
assert(not pcall(frame))
assert(not pcall(get_screen_width) and not pcall(get_screen_height))
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
  set_color {255, 0, 0, 255}
  draw_rect {x = 1, y = 1, width = 4, height = 4}
  frame()

  for _, invalid in ipairs({-1, 1.5, 1000001, math.huge, false}) do
    assert(not pcall(fade_off, invalid))
    assert(not pcall(fade_on, invalid))
  end
  assert(not mv.fade_active() and mv.fade_color()[4] == 0)
  assert(select("#", fade_off(2)) == 0)
  mv.fade_on(30)
  for tick, alpha in ipairs({0, 127, 255, 128, 0}) do
    assert(mv.fade_color()[4] == alpha)
    assert(mv.fade_active() == (tick < 5))
    if tick < 5 then mv.frame() end
  end
  fade_off()
  fade_on(nil)
  for tick = 0, 32 do
    local alpha = tick <= 16 and (255 * tick // 16) or (255 - 255 * (tick - 16) // 16)
    assert(mv.fade_color()[4] == alpha)
    assert(mv.fade_active() == (tick < 32))
    if tick < 32 then mv.frame() end
  end
  ran = true
end)
mv.start(function()
  local width, height = mv.window_size()
  assert(width == 321 and height == 234)
  assert(get_screen_width() == width and get_screen_height() == height)
  task()
end, {width = 321, height = 234})
assert(ran)
