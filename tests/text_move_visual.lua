-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local task = mv.process(function()
  local live_font = mv.font(24, 255, 0, 0)
  local text = mv.write(live_font, 10, 10, "Text")
  assert(not pcall(mv.change_text, text, "wrong", 0x100000000, 10, mv.font(24, 0, 255, 0)))
  assert(not pcall(mv.change_text, text, "wrong\0text", 20, 20))
  local centered = mv.write(mv.font(24, 0, 0, 255), 120, 70, "MMMM", "center")
  local right = mv.write(mv.font(24, 255, 255, 0), 220, 10, "MM", "right")
  for _ = 1, 15 do mv.frame() end
  -- Font tables stay live while native drawing owns the synchronized style.
  live_font[1], live_font[2], live_font[3], live_font.size = 0, 255, 255, 18
  collectgarbage("collect")
  for _ = 1, 5 do mv.frame() end
  mv.change_text(text, "Moved", 100, 40, mv.font(24, 0, 255, 0))
  for _ = 1, 15 do mv.frame() end
  mv.delete_text(text)
  mv.delete_text(centered)
  mv.delete_text(right)
end)
mv.start(function() task() end, {width = 240, height = 100, title = "Text move test"})
