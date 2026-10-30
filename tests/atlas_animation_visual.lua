-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local atlas
local task = mv.process(function()
  for tick = 1, 40 do
    local elapsed = tick < 16 and 0 or 100
    mv.draw_sprite(atlas, mv.animation_frame(atlas, "flash", elapsed), 10, 10)
    mv.frame()
  end
end)
mv.start(function()
  local image = mv.load_image("tests/fixtures/red.png")
  local file = assert(io.open("tests/fixtures/animated-atlas.json", "rb"))
  local json = file:read("a")
  file:close()
  atlas = mv.load_atlas(image, json)
  task()
end, {width = 64, height = 64, title = "Atlas animation test"})
