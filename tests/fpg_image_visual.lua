-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local actor = mv.process(function() while true do mv.frame() end end)
local producer = mv.process(function()
  local file = assert(io.open("tests/fixtures/fpg/bennugd-8.fpg", "rb"))
  local library = mv.fpg.parse(file:read("a"))
  file:close()
  local red = library:image(7, true)
  local green = library:image(42, true)
  assert(library:image(8) == nil)
  assert(not pcall(library.image, library, 7, 3))
  local width, height = mv.image_size(red)
  assert(width == 2 and height == 2)
  local world = mv.world.create()
  world:add_rect({x = 31, y = 21, width = 1, height = 1})
  local player = actor({graph = red, x = 20, y = 20, size = 10}, {attached = false})
  assert(world:collision(player) == 1)
  local weak = setmetatable({library, red}, {__mode = "v"})
  local drawer = mv.process(function()
    while true do
      mv.draw_image_region(green, 50, 20, 0, 0, 2, 1, 0, 0, 10)
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
  mv.clear_screen(25, 30, 45)
  producer()
end}
