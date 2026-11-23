-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0
require("malvado").import_globals()

local World = process(function()
  while not key("escape") do frame() end
end, {defaults = {graph = rect {width = 30, height = 30, color = {180, 80, 220}}}})

local Inventory = process(function(self)
  self.camera = false
  self.region = region {x = 80, y = 140, width = 480, height = 180}
  local offset = 0
  local colors = {{230, 80, 80}, {80, 210, 120}, {80, 150, 240}}
  while not key("escape") do
    if key("right") then offset = math.min(160, offset + 4) end
    if key("left") then offset = math.max(0, offset - 4) end
    self.scroll = scroll(offset, 0)
    for i = 0, 5 do
      local color = colors[i % #colors + 1]
      draw_rect {x = 100 + i * 100, y = 160, width = 80, height = 120, color = color}
    end
    frame()
  end
end)

local Overlay = process(function(self)
  self.camera = false
  while not key("escape") do
    draw_rect {x = 80, y = 140, width = 480, height = 180, color = WHITE, filled = false}
    draw_text {text = "Left / Right: scroll inventory  |  Escape: exit", x = 20, y = 20, size = 20}
    frame()
  end
end)

return {width = 640, height = 480, title = "Clipped screen-space inventory", init = function()
  clear_screen {25, 30, 45, 255}
  camera_set(200, 100)
  World({x = 800, y = 500})
  Inventory()
  Overlay({z = 10})
end}
