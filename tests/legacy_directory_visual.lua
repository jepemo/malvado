-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
require("malvado.compat.legacy").import_globals()
local library
local task = process(function(self)
  self.fpg = library
  self.x, self.y = 14, 14
  self.fpgIndex = 0
  for _ = 1, 15 do frame() end
  self.fpgIndex = 3
  for _ = 1, 20 do frame() end
end)
mv.start(function()
  library = fpg("tests/fixtures/legacy-colours")
  assert(not pcall(fpg, "tests/fixtures/missing-directory"))
  assert(not pcall(fpg, "tests"))
  assert(not pcall(fpg, "tests/fixtures/legacy-colours", "zip"))
  assert(not pcall(fpg, "tests/fixtures/legacy-invalid"))
  collectgarbage("collect")
  task()
end, {width = 64, height = 64, title = "Legacy directory test"})
