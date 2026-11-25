-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local font
local producer = mv.process(function()
  font = mv.load_font {path = "fixtures/fonts/AnonymousPro-Bold.ttf", size = 24, extra_glyphs = "ΩΩ"}
end)
local worker = mv.process(function()
  mv.frame()
  mv.write {font = font, text = "Café Ω", x = 100, y = 20, align = "center"}
  font = nil
  collectgarbage("collect")
  for _ = 1, 15 do mv.frame() end
end)
mv.start(function()
  assert(not pcall(mv.load_font, {path = "fixtures/fonts/missing.ttf", size = 24}))
  assert(not pcall(mv.load_font, {path = "fixtures/fonts/AnonymousPro-Bold.ttf", size = 0}))
  assert(not pcall(mv.load_font, {path = "fixtures/fonts/AnonymousPro-Bold.ttf", size = 24, extra_glyphs = "\xed\xa0\x80"}))
  assert(not pcall(mv.load_font, {path = "fixtures/fonts/AnonymousPro-Bold.ttf", size = 24, extra_glyphs = "\0"}))
  local too_many = {}
  for codepoint = 0x100, 0x441 do too_many[#too_many + 1] = utf8.char(codepoint) end
  assert(not pcall(mv.load_font, {path = "fixtures/fonts/AnonymousPro-Bold.ttf", size = 24, extra_glyphs = table.concat(too_many)}))
  assert(not pcall(mv.load_font, {path = "fixtures/fonts/AnonymousPro-Bold.ttf", size = 24, wrong = 1}))
  local otf = mv.load_font {path = "fixtures/fonts/SourceCodePro-Regular.otf", size = 24}
  assert(otf)
  local width, height = mv.text_size(otf, "OpenType")
  assert(width > 0 and height > 0)
  assert(select(1, mv.text_size(otf, "")) == 0 and select(2, mv.text_size(otf, "")) == 0)
  assert(mv.text_size(otf, "MMMM\nMMMM") > mv.text_size(otf, "MM"))
  assert(select(2, mv.text_size(otf, "MMMM\nMMMM")) > select(2, mv.text_size(otf, "MM")))
  producer()
  worker()
end, {width = 200, height = 80, title = "Font test"})
