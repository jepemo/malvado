-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

require("malvado").import_globals()

local WIDTH, HEIGHT = 640, 360

local Welcome = process(function()
    while not key(KEY_ESCAPE) do
        draw_text {text = "Dodge!", x = 20, y = 20, size = 30}
        frame()
    end
end)

return {
    title = "Dodge!",
    width = WIDTH,
    height = HEIGHT,
    init = function()
        Welcome()
    end,
}
