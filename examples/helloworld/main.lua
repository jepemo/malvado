-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

require("malvado").import_globals()

local HelloWorld = process(function()
  while not key(KEY_ESCAPE) do
    draw_text {text = "Hello World", x = 40, y = 40, size = 48}
    frame()
  end
end)

start(function()
  HelloWorld()
end, {title = "Hello world"})
