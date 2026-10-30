-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
require("malvado.compat.legacy").import_globals()
local image, sheet
local task = process(function(self)
  self.graph = image
  self.x, self.y = 14, 14
  frame()
  assert(self.width == 8 and self.height == 8)
  for _ = 1, 14 do frame() end
  self.fpg = sheet
  self.fpgIndex = 3
  self.x, self.y = 10, 10
  frame()
  assert(self.width == 0 and self.height == 0)
  for _ = 1, 19 do frame() end
end)
mv.start(function()
  image = load_image("tests/fixtures/red.png")
  sheet = fpg("tests/fixtures/red.png", "image", 1, 2)
  assert(not pcall(fpg, "tests/fixtures/red.png", "image", 3, 2))
  assert(not pcall(fpg, "tests/fixtures/red.png", "directory"))
  assert(not pcall(fpg, "tests/fixtures/red.png", "image\0x", 1, 2))
  task()
end, {width = 64, height = 64, title = "Legacy graphics test"})
