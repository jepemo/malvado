-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

require("malvado").import_globals()

local images = {}

local Cat = process(function(self)
  local image = 1
  self.fps = 15
  while not key("escape") do
    draw_image(images[image], self.x, self.y)
    image = image % #images + 1
    frame()
  end
end)

start(function()
  set_title("Sprites")
  clear_screen(25, 30, 45)
  for i = 1, 6 do
    images[i] = load_image("examples/demo/assets/cat/c" .. i .. ".png")
  end
  Cat { x = 360, y = 260 }
end)
