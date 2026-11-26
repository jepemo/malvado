-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")

local function report()
    local color = mv.fade_color()
    print(("Overlay: %d %d %d %d"):format(table.unpack(color)))
end

local transition = mv.process(function()
    report()
    mv.fade_off(250)
    mv.fade_to {color = {0, 0, 64}, duration_ms = 250}
    mv.fade_on(250)
    assert(mv.fade_active())
    local saw_blue = false
    while mv.fade_active() do
        local color = mv.fade_color()
        if not saw_blue and color[3] == 64 and color[4] == 255 then
            saw_blue = true
            print("Reached opaque blue")
        end
        mv.frame()
    end
    assert(saw_blue)
    report()
    print("Fade queue finished")
end)

mv.start(function() transition() end)
