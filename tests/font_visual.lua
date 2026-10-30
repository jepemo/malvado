-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local worker = mv.process(function()
  for _ = 1, 15 do mv.frame() end
end)
mv.start(function()
  assert(not pcall(mv.load_font, "tests/fixtures/fonts/missing.ttf", 24))
  assert(not pcall(mv.load_font, "tests/fixtures/fonts/AnonymousPro-Bold.ttf", 0))
  assert(not pcall(mv.load_font, "tests/fixtures/fonts/AnonymousPro-Bold.ttf", 24, 256))
  assert(not pcall(mv.load_font, "tests/fixtures/fonts/AnonymousPro-Bold.ttf", 24, nil, nil, nil, nil, "\xed\xa0\x80"))
  assert(not pcall(mv.load_font, "tests/fixtures/fonts/AnonymousPro-Bold.ttf", 24, nil, nil, nil, nil, "\0"))
  local too_many = {}
  for codepoint = 0x100, 0x441 do too_many[#too_many + 1] = utf8.char(codepoint) end
  assert(not pcall(mv.load_font, "tests/fixtures/fonts/AnonymousPro-Bold.ttf", 24, nil, nil, nil, nil, table.concat(too_many)))
  local font = mv.load_font("tests/fixtures/fonts/AnonymousPro-Bold.ttf", 24, 255, 255, 255, 255, "ΩΩ")
  mv.write(font, 100, 20, "Café Ω", "center")
  font = nil
  collectgarbage("collect")
  worker()
end, {width = 200, height = 80, title = "Font test"})
