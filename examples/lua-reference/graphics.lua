-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")

local clip = mv.region {x = 20, y = 20, width = 120, height = 80}
local actor = mv.process(function(self)
    for offset = 0, 119 do
        self.scroll = mv.scroll(offset, 0)
        mv.frame()
    end
end)

mv.start(function()
    actor {x = 135, y = 48, region = clip,
           graph = mv.rect {width = 50, height = 28, pivot = "center", color = mv.colors.BLUE}}
    actor {x = 155, y = 78, region = clip,
           graph = mv.circle {radius = 16, color = mv.colors.ORANGE}}
end, {title = "Clipped process graphics", width = 180, height = 120})
