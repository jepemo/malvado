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
  local font = mv.load_font("tests/fixtures/fonts/AnonymousPro-Bold.ttf", 24, 255, 255, 255)
  mv.write(font, 100, 20, "Café", "center")
  font = nil
  collectgarbage("collect")
  worker()
end, {width = 200, height = 80, title = "Font test"})
