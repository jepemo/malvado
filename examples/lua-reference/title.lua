-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")

local show = mv.process(function()
    for _ = 1, 120 do
        mv.draw_text {text = "Read the window title", x = 20, y = 20}
        mv.frame()
    end
end)

mv.start(function()
    mv.set_title("Level 2")
    show()
end, {title = "Loading", width = 320, height = 120})
