-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")

local show = mv.process(function(self)
    print(("Process cadence: %d"):format(self.fps))
    mv.frame()
end)

local began_at = mv.time_seconds()
mv.start(function()
    local width, height = mv.window_size()
    print(("Initial size: %dx%d"):format(width, height))
    show()
    mv.set_fps(20)
    show()
    mv.set_window_size(640, 360)
    width, height = mv.window_size()
    print(("New size: %dx%d"):format(width, height))
end, {width = 320, height = 240, fps = 30})

print(("Elapsed wall time: %.6f seconds"):format(mv.time_seconds() - began_at))
