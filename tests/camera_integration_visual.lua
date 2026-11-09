-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
mv.import_globals()
local explicit, blue
local world = world_create()
world:add_rect({x = 67, y = 47, width = 1, height = 1})
local Actor = process(function(self)
  if self.physical then
    assert(self.x == 70 and self.y == 50 and world:collision(self) == 1)
  end
  while true do
    if self.paint then
      set_color(0, 255, 0)
      draw_box(70, 50, 74, 54)
    end
    frame()
  end
end)
local Inventory = process(function(self)
  self.camera = false
  self.region = region(20, 90, 30, 20)
  self.scroll = scroll(20, 0)
  while true do
    set_color(255, 255, 0)
    draw_box(40, 90, 80, 110)
    frame()
  end
end)
local Overlay = process(function(self)
  self.camera = false
  while true do
    set_color(255, 255, 255)
    draw_box(140, 100, 150, 110)
    frame()
  end
end)
local Control = process(function()
  local tick = 1
  while true do
    if tick == 10 then explicit:destroy() end
    if tick == 20 then
      blue.camera = false
      blue.x, blue.y = 45, 65
      blue.scroll = nil
    end
    if tick == 30 then fade_to(0, 255, 255, 128, 0) end
    tick = tick + 1
    frame()
  end
end)
return {width = 160, height = 120, init = function()
  clear_screen(0, 0, 0)
  camera_set(50, 30)
  Actor({x = 70, y = 50, physical = true, paint = true,
    graph = rect(20, 20, {color = {255, 0, 0}}),
    scroll = scroll(10, 5), region = region(5, 10, 10, 10)})
  explicit = camera_create(100, 50, {viewport = region(40, 60, 50, 40)})
  blue = Actor({x = 110, y = 60, camera = explicit,
    graph = circle(20, {color = {0, 0, 255}}),
    scroll = scroll(5, 5), region = region(45, 65, 20, 20)})
  -- Empty viewport/region intersection and wholly off-screen viewport.
  Actor({x = 110, y = 60, camera = explicit, region = region(0, 0, 10, 10),
    graph = circle(30, {color = {0, 255, 0}})})
  Actor({x = 0, y = 0, camera = camera_create(0, 0, {viewport = region(-100, -100, 20, 20)}),
    graph = rect(1000, 1000, {color = {0, 255, 0}})})
  Actor({x = 130, y = 50, graph = rect(10, 10, {color = {255, 0, 0}})})
  Actor({x = 80, y = 20, camera = false, z = 10,
    graph = rect(10, 10, {color = {0, 255, 0}})})
  Inventory({z = 15})
  Overlay({z = 20})
  write(font(16, 255, 0, 255), 110, 5, "I")
  Control()
end}
