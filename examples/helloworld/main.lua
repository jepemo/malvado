-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")

local HelloWorld = mv.process(function(self)
  local font = mv.font(60, 0, 0, 0)
  mv.write(font, self.x, self.y, self.text)

  while not mv.key("escape") do
    mv.frame()
  end
end)

mv.start(function()
  mv.set_title("Hello world")
  mv.clear_screen(250, 250, 250)
  HelloWorld { x = 400, y = 300, text = "Hello World" }
end)
