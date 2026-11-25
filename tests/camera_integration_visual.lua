-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
mv.import_globals()
local explicit, blue, image
local world = world_create()
world_add_rect(world, {x = 67, y = 47, width = 1, height = 1})
local Actor = process(function(self)
  if self.physical then
    assert(self.x == 70 and self.y == 50 and world_collision(world, self) == 1)
  end
  while true do
    if self.paint then
      draw_image {
        image = image, x = 70, y = 50,
        source = {x = 4, y = 0, width = 4, height = 4}, scale = 2
      }
      draw_line {x1 = 60, y1 = 53, x2 = 82, y2 = 53, color = YELLOW}
    end
    frame()
  end
end)
local Inventory = process(function(self)
  self.camera = false
  self.region = region {x = 20, y = 90, width = 30, height = 20}
  self.scroll = scroll(20, 0)
  while true do
    set_color {255, 255, 0, 255}
    draw_rect {x = 40, y = 90, width = 40, height = 20}
    frame()
  end
end)
local Overlay = process(function(self)
  self.camera = false
  while true do
    set_color {255, 255, 255, 255}
    draw_rect {x = 140, y = 100, width = 10, height = 10}
    frame()
  end
end)
local Control = process(function()
  local tick = 1
  while true do
    if tick == 10 then camera_destroy(explicit) end
    if tick == 20 then
      blue.camera = false
      blue.x, blue.y = 45, 65
      blue.scroll = nil
    end
    if tick == 30 then fade_to {color = {0, 255, 255, 128}, duration_ms = 0} end
    tick = tick + 1
    frame()
  end
end)
return {width = 160, height = 120, init = function()
  clear_screen {0, 0, 0, 255}
  camera_set(50, 30)
  image = load_image("fixtures/red.png")
  Actor({x = 70, y = 50, physical = true, paint = true,
    graph = rect {width = 20, height = 20, color = {255, 0, 0}},
    scroll = scroll(10, 5), region = region {x = 5, y = 10, width = 10, height = 10}})
  explicit = camera_create {x = 100, y = 50, viewport = region {x = 40, y = 60, width = 50, height = 40}}
  blue = Actor({x = 110, y = 60, camera = explicit,
    graph = circle {radius = 20, color = {0, 0, 255}},
    scroll = scroll(5, 5), region = region {x = 45, y = 65, width = 20, height = 20}})
  -- Empty viewport/region intersection and wholly off-screen viewport.
  Actor({x = 110, y = 60, camera = explicit, region = region {x = 0, y = 0, width = 10, height = 10},
    graph = circle {radius = 30, color = {0, 255, 0}}})
  Actor({x = 0, y = 0, camera = camera_create {x = 0, y = 0, viewport = region {x = -100, y = -100, width = 20, height = 20}},
    graph = rect {width = 1000, height = 1000, color = {0, 255, 0}}})
  Actor({x = 130, y = 50, graph = rect {width = 10, height = 10, color = {255, 0, 0}}})
  Actor({x = 80, y = 20, camera = false, z = 10,
    graph = rect {width = 10, height = 10, color = {0, 255, 0}}})
  Inventory({z = 15})
  Overlay({z = 20})
  write {font = font {size = 16}, color = {255, 0, 255, 255}, text = "I", x = 110, y = 5}
  Control()
end}
