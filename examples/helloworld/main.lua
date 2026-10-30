-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

require("malvado").import_globals()

local HelloWorld = process(function(self)
  local font = font(60, 0, 0, 0)
  write(font, self.x, self.y, self.text)

  while not key("escape") do
    frame()
  end
end)

start(function()
  set_title("Hello world")
  clear_screen(250, 250, 250)
  HelloWorld { x = 400, y = 300, text = "Hello World" }
end)
