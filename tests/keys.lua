-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local process = mv.process(function()
    for _, name in ipairs({"space", "escape", "left", "right", "up", "down", "enter", "tab", "backspace", "a", "z", "0", "9"}) do
        assert(not mv.key(name))
    end
    for _, name in ipairs({"unknown", "", "A", "10", "space\0x"}) do
        assert(not pcall(mv.key, name))
    end
end)
mv.start(function() process() end)
