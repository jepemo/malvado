-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local image, atlas, creator
local weak = setmetatable({}, {__mode = "v"})
local producer = mv.process(function()
  image = mv.load_image("tests/fixtures/red.png")
  local file = assert(io.open("tests/fixtures/atlas.json", "rb"))
  atlas = mv.load_atlas(image, file:read("a"))
  file:close()
  weak[1], weak[2] = image, atlas
end)
local consumer = mv.process(function()
  mv.frame()
  assert(creator.status == "dead")
  collectgarbage("collect")
  assert(weak[1] and weak[2])
  mv.draw_image(image, 10, 10)
  mv.draw_sprite(atlas, 7, 30, 10)
  image, atlas = nil, nil
  collectgarbage("collect")
  assert(weak[1]) -- The queued drawing retains the image.
  mv.frame()
  for _ = 1, 15 do
    mv.draw_image(weak[1], 10, 10)
    mv.frame()
  end
  collectgarbage("collect")
  collectgarbage("collect")
  assert(weak[1] == nil and weak[2] == nil)
end)
mv.start(function() creator = producer(); consumer() end,
  {width = 64, height = 64, title = "Resource lifetime"})
