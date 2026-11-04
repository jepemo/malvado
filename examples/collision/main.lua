-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

require("malvado").import_globals()

local width, height = 640, 480
local radius, speed = 20, 4
local square = {x = 400, y = 180, size = 100}

local function circle_hits_square(x, y)
  local closest_x = math.max(square.x, math.min(x, square.x + square.size))
  local closest_y = math.max(square.y, math.min(y, square.y + square.size))
  local dx, dy = x - closest_x, y - closest_y
  return dx * dx + dy * dy <= radius * radius
end

local Square = process(function()
  while not key("escape") do
    set_color(255, 165, 70)
    draw_box(square.x, square.y, square.x + square.size, square.y + square.size)
    frame()
  end
end)

local Circle = process(function()
  local x, y = 120, 230
  while not key("escape") do
    if key("left") then x = x - speed end
    if key("right") then x = x + speed end
    if key("up") then y = y - speed end
    if key("down") then y = y + speed end
    x = math.max(radius, math.min(width - radius, x))
    y = math.max(radius, math.min(height - radius, y))

    local hit = circle_hits_square(x, y)
    set_color(80, 190, 255)
    draw_fcircle(x, y, radius)
    draw_text("Arrow keys: move  |  Escape: exit", 20, 20, 20)
    if hit then
      draw_text("Collision!", 20, 55, 28)
    end
    frame()
  end
end)

start(function()
  clear_screen(25, 30, 45)
  Square()
  Circle()
end, {width = width, height = height, title = "Circle collision"})
