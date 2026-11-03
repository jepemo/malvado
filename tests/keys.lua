-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
mv.import_globals()
for code = string.byte("A"), string.byte("Z") do
    local upper = string.char(code)
    local lower = upper:lower()
    assert(mv.input["KEY_" .. upper] == lower and _G["KEY_" .. upper] == lower)
    assert(mv.key(lower) == mv.input.key_down(_G["KEY_" .. upper]))
end
for code = string.byte("0"), string.byte("9") do
    local digit = string.char(code)
    assert(mv.input["KEY_" .. digit] == digit and _G["KEY_" .. digit] == digit)
    assert(mv.key(digit) == mv.input.key_down(_G["KEY_" .. digit]))
end
for _, name in ipairs({"escape", "space", "left", "right", "up", "down", "enter", "tab", "backspace"}) do
    local constant = "KEY_" .. name:upper()
    assert(mv.input[constant] == name and _G[constant] == name)
    assert(mv.key(name) == mv.input.key_down(_G[constant]))
end
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
    assert(mv.text_input_next() == nil)
end)
mv.start(function() process() end)
