-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local Actor = mv.process(function(self)
  while true do
    mv.set_color {0, 255, 0, 255}
    mv.draw_rect {x = 10, y = 10, width = 20, height = 20}
    local image = mv.load_image("tests/fixtures/red.png")
    mv.draw_image {image = image, x = 10, y = 10}
    mv.draw_circle {x = 12, y = 12, radius = 1}
    mv.draw_image {image = image, x = 20, y = 10, source = {x = 0, y = 0, width = 2, height = 2}}
    mv.draw_text {text = "Native", x = 2, y = 40, size = 10}
    mv.draw_line {x1 = 8, y1 = 18, x2 = 28, y2 = 18, color = mv.colors.YELLOW}
    mv.draw_line {x1 = 35, y1 = 10, x2 = 35, y2 = 25}
    mv.draw_line {x1 = 40, y1 = 10, x2 = 54, y2 = 24, thickness = 4, color = {255, 0, 0, 128}}
    mv.draw_line {x1 = 40, y1 = 30, x2 = 40, y2 = 30, color = mv.colors.WHITE}
    image = nil
    collectgarbage("collect")
    mv.frame()
  end
end)
mv.start(function()
  Actor {x = 10, y = 10, fps = 5,
    graph = mv.graphics.rect {width = 20, height = 20, pivot = "top-left", color = mv.colors.BLUE}}
end, {width=64,height=64,title="Shared native process commands"})
