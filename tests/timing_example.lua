-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local frame = mv.frame
local ticks = 0
mv.frame = function()
    ticks = ticks + 1
    frame()
end
mv.key = function(key)
    return (key == "space" and ticks == 4) or (key == "escape" and ticks >= 10)
end
dofile("examples/c/timing-game/main.lua")
assert(ticks >= 10)
