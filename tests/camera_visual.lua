-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0
require("malvado").import_globals()
local world = world_create()
world:add_rect({x = 67, y = 47, width = 1, height = 1})
local Actor = process(function(self)
  if self.physical then
    assert(self.x == 70 and self.y == 50 and world:collision(self) == 1)
  end
  while true do frame() end
end)
local Drawer = process(function()
  while true do
    set_color(255, 255, 0)
    draw_box(80, 100, 90, 110)
    frame()
  end
end)
return {width = 160, height = 120, init = function()
  clear_screen(0, 0, 0)
  camera_set(40, 20)
  camera_move(10, 10)
  Actor({x = 70, y = 50, physical = true, graph = rect(10, 10, {color = {255, 0, 0}})})
  local camera = camera_create(100, 50, {viewport = region(40, 60, 50, 40)})
  Actor({x = 110, y = 60, camera = camera, graph = circle(20, {color = {0, 0, 255}})})
  Actor({x = 140, y = 20, camera = false, z = 10, graph = rect(10, 10, {color = {0, 255, 0}})})
  Drawer()
end}
