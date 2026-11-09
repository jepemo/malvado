-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0
require("malvado").import_globals()
local explicit
local Player = process(function(self)
  self.graph = circle(8, {color = {255, 0, 0}})
  camera_follow(self)
  while true do
    self.x = self.x + 5
    frame()
  end
end)
local Other = process(function(self)
  self.graph = circle(5, {color = {0, 0, 255}})
  explicit:follow(self)
  while true do
    self.x = self.x - 3
    frame()
  end
end)
local HUD = process(function(self)
  self.camera = false
  self.graph = rect(10, 10, {color = {0, 255, 0}})
  while true do frame() end
end)
return {width = 160, height = 120, init = function()
  clear_screen(0, 0, 0)
  explicit = camera_create(0, 0, {viewport = region(40, 60, 50, 40)})
  Player({x = 100, y = 90})
  Other({x = 300, y = 100, camera = explicit})
  HUD({x = 140, y = 20, z = 10})
end}
