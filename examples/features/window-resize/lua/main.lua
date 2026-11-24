-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")

local function print_size()
    local width, height = mv.window_size()
    print(("window: %dx%d"):format(width, height))
end

local resize = mv.process(function()
    for _ = 1, 60 do mv.frame() end
    mv.set_window_size(640, 360)
    -- Give the window system time to process the request before reading back.
    for _ = 1, 60 do mv.frame() end
    print_size()
end)

return {
    title = "Malvado Window Resize",
    width = 320,
    height = 240,
    init = function()
        mv.clear_screen({20, 30, 40})
        print_size()
        resize()
    end,
}
