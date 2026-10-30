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
    local x, y = mv.mouse_position()
    assert(x == 0 and y == 0)
    for _, name in ipairs({"left", "middle", "right"}) do
        assert(not mv.mouse_button(name))
    end
    for _, name in ipairs({"other", "Left", "left\0x"}) do
        assert(not pcall(mv.mouse_button, name))
    end
end)
mv.start(function() process() end)
