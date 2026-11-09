-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0
require("malvado").import_globals()

local Wall = process(function()
  while not key("escape") do frame() end
end, {defaults = {graph = rect(20, 120, {color = {255, 165, 70}})}})

local Player = process(function(self)
  self.graph = circle(16, {color = {80, 190, 255}})
  camera_follow(self)
  while not key("escape") do
    if key("left") then self.x = self.x - 3 end
    if key("right") then self.x = self.x + 3 end
    if key("up") then self.y = self.y - 3 end
    if key("down") then self.y = self.y + 3 end
    frame()
  end
end)

local HUD = process(function(self)
  self.camera = false
  while not key("escape") do
    draw_text("Arrow keys: move  |  Escape: exit", 20, 20, 20)
    frame()
  end
end)

start(function()
  clear_screen(25, 30, 45)
  Wall({x = 500, y = 240})
  Player({x = 320, y = 240})
  HUD({z = 10})
end, {width = 640, height = 480, title = "Camera following"})
