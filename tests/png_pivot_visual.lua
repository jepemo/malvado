-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
mv.import_globals()
local path = assert(os.getenv("MALVADO_PNG_PIVOT_TEST_FILE"))
local world = world_create()
world:add_rect({x = 5, y = 5, width = 1, height = 1})
world:add_rect({x = 90, y = 20, width = 1, height = 1})
world:add_rect({x = 40, y = 50, width = 1, height = 1})
local Actor = process(function(self)
  local width, height = image_size(self.graph)
  assert(width == 5 and height == 3)
  assert(world:collision(self) == self.hit)
  local automatic = world:collision(self)
  self.collider = false
  assert(world:collision(self) == nil)
  self.collider = mv.collision.circle(0.1, {offset_x = 100})
  assert(world:collision(self) == nil)
  self.collider = nil
  assert(world:collision(self) == automatic)
  while true do
    collectgarbage("collect")
    assert(self.graph)
    frame()
  end
end)
local Producer = process(function()
  assert(not pcall(load_asset, path, {pivot = "other"}))
  assert(not pcall(load_asset, path, {pivot = "center\0hidden"}))
  local centered = load_asset(path, {pivot = "center"})
  local default = load_asset(path)
  local top_left = load_asset(path, {format = "png", pivot = "top-left"})
  Actor({graph = centered, x = 30, y = 20, size = 10, hit = 1}, {attached = false})
  Actor({graph = default, x = 90, y = 20, size = 10, hit = 2}, {attached = false})
  Actor({graph = centered, x = 30, y = 70, size = 10, angle = 90, hit = 3}, {attached = false})
  local Drawer = process(function()
    while true do
      draw_image(centered, 90, 80) -- Explicit draw_image keeps its top-left coordinates.
      draw_image(top_left, 100, 80)
      frame()
    end
  end)
  Drawer({}, {attached = false})
end)
return {width = 160, height = 120, init = function()
  clear_screen(0, 0, 0)
  Producer()
end}
