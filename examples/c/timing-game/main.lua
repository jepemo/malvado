-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

require("malvado").import_globals()
local Game = process(function()
    local elapsed, stopped = 0, nil
    while not key("escape") do
        if not stopped then
            elapsed = elapsed + 1
            if key("space") then stopped = elapsed end
        end
        draw_text("Press SPACE as close to 5 seconds as you can.", 30, 40, 20)
        draw_text(string.format("Time: %.2f seconds", elapsed / 60), 30, 100, 28)
        if stopped then
            draw_text(string.format("Difference: %.2f seconds. Press ESC to exit.",
                math.abs(stopped - 300) / 60), 30, 160, 20)
        end
        frame()
    end
end)
start(function()
    clear_screen(25, 30, 45)
    Game()
end, {width = 640, height = 360, title = "Timing game"})
