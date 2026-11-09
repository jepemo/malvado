-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
mv.import_globals()

local world = world_create()
local player_graphic = mv.graphics.circle(16, {color = {80, 190, 255}})
world:add_rect({x = 300, y = 100, width = 40, height = 280})

local Player = process(function(self)
  self.world = world
  while not key("escape") do
    local dx, dy = 0, 0
    if key("left") then dx = -4 end
    if key("right") then dx = 4 end
    if key("up") then dy = -4 end
    if key("down") then dy = 4 end
    world_move(dx, dy)

    set_color(255, 165, 70)
    draw_box(300, 100, 340, 380)
    draw_text("Arrow keys: move  |  Escape: exit", 20, 20, 20)
    frame()
  end
end, {defaults = {graph = player_graphic}})

start(function()
  clear_screen(25, 30, 45)
  Player({x = 100, y = 200})
end, {width = 640, height = 480, title = "World collision"})
