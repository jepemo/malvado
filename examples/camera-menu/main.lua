-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0
require("malvado").import_globals()

local World = process(function()
  while not key("escape") do frame() end
end, {defaults = {graph = rect(30, 30, {color = {180, 80, 220}})}})

local Inventory = process(function(self)
  self.camera = false
  self.region = region(80, 140, 480, 180)
  local offset = 0
  local colors = {{230, 80, 80}, {80, 210, 120}, {80, 150, 240}}
  while not key("escape") do
    if key("right") then offset = math.min(160, offset + 4) end
    if key("left") then offset = math.max(0, offset - 4) end
    self.scroll = scroll(offset, 0)
    for i = 0, 5 do
      local color = colors[i % #colors + 1]
      set_color(color[1], color[2], color[3])
      draw_box(100 + i * 100, 160, 180 + i * 100, 280)
    end
    frame()
  end
end)

local Overlay = process(function(self)
  self.camera = false
  while not key("escape") do
    set_color(255, 255, 255)
    draw_rect(80, 140, 560, 320)
    draw_text("Left / Right: scroll inventory  |  Escape: exit", 20, 20, 20)
    frame()
  end
end)

return {width = 640, height = 480, title = "Clipped screen-space inventory", init = function()
  clear_screen(25, 30, 45)
  camera_set(200, 100)
  World({x = 800, y = 500})
  Inventory()
  Overlay({z = 10})
end}
