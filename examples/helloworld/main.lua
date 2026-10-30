-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")

local HelloWorld = mv.process(function(self)
  local font = mv.font(60, 0, 0, 0)
  local greeting = mv.write(font, self.x, self.y, self.text)
  local smoke = os.getenv("MALVADO_SMOKE") == "1"
  local ticks = 0

  while not mv.key("escape") do
    ticks = ticks + 1
    if smoke and ticks >= 3 then break end
    mv.frame()
  end

  mv.delete_text(greeting)
end)

mv.start(function()
  mv.clear_screen(250, 250, 250)
  HelloWorld { x = 400, y = 300, text = "Hello World" }
end, { width = 800, height = 600, title = "Hello world" })
