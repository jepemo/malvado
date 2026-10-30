-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local image
local task = mv.process(function()
  for tick = 1, 8 do
    local x = tick <= 4 and 0 or 4
    mv.draw_image_region(image, 10, 10, x, 0, 4, 8)
    mv.frame()
  end
end)
mv.start(function()
  image = mv.load_image("tests/fixtures/red.png")
  task {fps = 15}
end, {width = 64, height = 64, title = "Cadence test"})
