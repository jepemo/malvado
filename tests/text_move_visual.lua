-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local task = mv.process(function()
  local f = mv.font {size = 24}
  local text = mv.write {font = f, text = "Text", x = 10, y = 10, color = {255, 0, 0, 255}}
  assert(not pcall(mv.change_text, text, {text = "wrong", x = 0x100000000}))
  assert(not pcall(mv.change_text, text, {text = "wrong\0text"}))
  local centered = mv.write {font = f, text = "MMMM", x = 120, y = 70, align = "center", color = {0, 0, 255, 255}}
  local right = mv.write {font = f, text = "MM", x = 220, y = 10, align = "right", color = {255, 255, 0, 255}}
  for _ = 1, 15 do mv.frame() end
  -- Per-text style updates are visible without recreating fonts.
  mv.change_text(text, {color = {0, 255, 255, 255}, size = 18})
  collectgarbage("collect")
  for _ = 1, 5 do mv.frame() end
  mv.change_text(text, {text = "Moved", x = 100, y = 40, color = {0, 255, 0, 255}})
  for _ = 1, 15 do mv.frame() end
  mv.delete_text(text)
  mv.delete_text(centered)
  mv.delete_text(right)
end)
mv.start(function() task() end, {width = 240, height = 100, title = "Text move test"})
