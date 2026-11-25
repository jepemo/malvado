-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0
require("malvado").import_globals()

local Wall = process(function()
  while not key(KEY_ESCAPE) do frame() end
end, {defaults = {graph = rect {width = 20, height = 120, color = {255, 165, 70}}}})

local Player = process(function(self)
  self.graph = circle {radius = 16, color = {80, 190, 255}}
  camera_follow(self)
  while not key(KEY_ESCAPE) do
    if key(KEY_LEFT) then self.x = self.x - 3 end
    if key(KEY_RIGHT) then self.x = self.x + 3 end
    if key(KEY_UP) then self.y = self.y - 3 end
    if key(KEY_DOWN) then self.y = self.y + 3 end
    frame()
  end
end)

local HUD = process(function(self)
  self.camera = false
  while not key(KEY_ESCAPE) do
    draw_text {text = "Arrow keys: move  |  Escape: exit", x = 20, y = 20, size = 20}
    frame()
  end
end)

start(function()
  clear_screen {25, 30, 45, 255}
  Wall({x = 500, y = 240})
  Player({x = 320, y = 240})
  HUD({z = 10})
end, {width = 640, height = 480, title = "Camera following"})
