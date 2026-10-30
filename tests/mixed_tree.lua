-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local child = mv.process(function()
    error("The child must not run after the tree is put to sleep")
end)
local root = mv.process(function()
    local guard <close> = setmetatable({}, {
        __close = function() print("Lua root closed") end,
    })
    child()
    mv.frame()
    error("The root must not resume after the tree is put to sleep")
end)
mv.start(function() root() end)
