-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local image
local Red = mv.process(function(self)
  assert(self.z == 10)
  assert(not pcall(function() self.z = 0 / 0 end))
  for tick = 1, 40 do
    if tick == 16 then self.z = -1 end
    if tick == 31 then self.z = 0 end
    mv.draw_image_region(image, 10, 10, 0, 0, 4, 8)
    mv.frame()
  end
end)
local Green = mv.process(function(self)
  assert(self.z == 0)
  for _ = 1, 40 do
    mv.draw_image_region(image, 10, 10, 4, 0, 4, 8)
    mv.frame()
  end
end)
mv.start(function()
  image = mv.load_image("tests/fixtures/red.png")
  assert(not pcall(Red, {z = math.huge}))
  Red {z = 10}
  Green()
end, {width = 64, height = 64, title = "Draw order test"})
