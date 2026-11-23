-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0
require("malvado").import_globals()
local world = world_create()
world_add_rect(world, {x = 67, y = 47, width = 1, height = 1})
local Actor = process(function(self)
  if self.physical then
    assert(self.x == 70 and self.y == 50 and world_collision(world, self) == 1)
  end
  while true do frame() end
end)
local Drawer = process(function()
  while true do
    set_color {255, 255, 0, 255}
    draw_rect {x = 80, y = 100, width = 10, height = 10}
    frame()
  end
end)
return {width = 160, height = 120, init = function()
  clear_screen {0, 0, 0, 255}
  camera_set(40, 20)
  camera_move(10, 10)
  Actor({x = 70, y = 50, physical = true, graph = rect {width = 10, height = 10, color = {255, 0, 0}}})
  local camera = camera_create {x = 100, y = 50, viewport = region {x = 40, y = 60, width = 50, height = 40}}
  Actor({x = 110, y = 60, camera = camera, graph = circle {radius = 20, color = {0, 0, 255}}})
  Actor({x = 140, y = 20, camera = false, z = 10, graph = rect {width = 10, height = 10, color = {0, 255, 0}}})
  Drawer()
end}
