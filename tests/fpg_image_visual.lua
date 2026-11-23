-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local actor = mv.process(function() while true do mv.frame() end end)
local producer = mv.process(function()
  local file = assert(io.open("tests/fixtures/fpg/bennugd-8.fpg", "rb"))
  local library = mv.fpg.parse(file:read("a"))
  file:close()
  local red = mv.fpg_image(library, 7, true)
  local green = mv.fpg_image(library, 42, true)
  assert(mv.fpg_image(library, 8) == nil)
  assert(not pcall(mv.fpg_image, library, 7, 3))
  local width, height = mv.image_size(red)
  assert(width == 2 and height == 2)
  local world = mv.world_create()
  mv.world_add_rect(world, {x = 31, y = 21, width = 1, height = 1})
  local player = actor({graph = red, x = 20, y = 20, size = 10}, {attached = false})
  assert(mv.world_collision(world, player) == 1)
  local weak = setmetatable({library, red}, {__mode = "v"})
  local drawer = mv.process(function()
    while true do
      mv.draw_image {image = green, x = 50, y = 20, source = {x = 0, y = 0, width = 2, height = 1}, scale = 10}
      mv.frame()
    end
  end)
  drawer({}, {attached = false})
  library, red = nil, nil
  collectgarbage("collect")
  collectgarbage("collect")
  assert(not weak[1] and weak[2])
end)
return {width = 128, height = 80, init = function()
  mv.clear_screen {25, 30, 45, 255}
  producer()
end}
