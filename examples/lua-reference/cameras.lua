-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local view = mv.camera_create {
    viewport = mv.region {x = 0, y = 0, width = 100, height = 80},
}

local actor = mv.process(function(self)
    view:set(10, 20)
    view:move(5, -5)
    local x, y = view:position()
    assert(x == 15 and y == 15)
    print("Camera moved to 15, 15")

    view:follow(self)
    mv.frame() -- Following updates after the process yields.
    x, y = view:position()
    assert(x == 70 and y == 40)
    print("Camera followed to 70, 40")

    view:destroy()
    assert(not pcall(mv.camera_position, view))
end)

mv.start(function()
    actor {x = 120, y = 80, camera = view}
end)
