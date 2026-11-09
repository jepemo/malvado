-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
mv.import_globals()

local Scene = process(function(self)
  local camera_x, camera_y = 80, 40
  while not key("escape") do
    if key("left") then camera_x = camera_x - 3 end
    if key("right") then camera_x = camera_x + 3 end
    if key("up") then camera_y = camera_y - 3 end
    if key("down") then camera_y = camera_y + 3 end
    self.scroll = scroll(camera_x, camera_y)
    set_color(255, 165, 70)
    draw_box(280, 180, 340, 240)
    frame()
  end
end, {defaults = {
  graph = rect(80, 60, {pivot = "top-left", color = {80, 190, 255}}),
  region = region(40, 60, 300, 240)
}})

local Overlay = process(function()
  while not key("escape") do
    set_color(240, 240, 240)
    draw_rect(40, 60, 340, 300)
    draw_text("Arrow keys: scroll  |  Escape: exit", 20, 20, 20)
    frame()
  end
end)

local game = {width = 640, height = 480, title = "Process scrolling"}
function game.init()
  clear_screen(25, 30, 45)
  Scene({x = 140, y = 130})
  Overlay({z = 1})
end
return game
