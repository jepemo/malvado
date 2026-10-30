-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local Game = mv.process(function()
    local elapsed, stopped = 0, nil
    while not mv.key("escape") do
        if not stopped then
            elapsed = elapsed + 1
            if mv.key("space") then stopped = elapsed end
        end
        mv.draw_text("Press SPACE as close to 5 seconds as you can.", 30, 40, 20)
        mv.draw_text(string.format("Time: %.2f seconds", elapsed / 60), 30, 100, 28)
        if stopped then
            mv.draw_text(string.format("Difference: %.2f seconds. Press ESC to exit.",
                math.abs(stopped - 300) / 60), 30, 160, 20)
        end
        mv.frame()
    end
end)
mv.start(function()
    mv.clear_screen(25, 30, 45)
    Game()
end, {width = 640, height = 360, title = "Timing game"})
