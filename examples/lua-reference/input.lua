-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local input = mv.input

local show = mv.process(function()
    local keyboard_open = false
    local typed = ""
    for _ = 1, 900 do
        if input.key_pressed(mv.KEY_ENTER) then
            keyboard_open = not keyboard_open
            print("On-screen keyboard request accepted:", input.text_input_set_keyboard(keyboard_open))
        end

        local character = input.text_input_next()
        while character do
            typed = typed .. character
            character = input.text_input_next()
        end

        if input.key_pressed(mv.KEY_SPACE) then print("Space pressed") end
        if input.key_released(mv.KEY_SPACE) then print("Space released") end

        local x, y = input.mouse_position()
        if input.mouse_pressed(mv.MOUSE_LEFT) then print("Left pressed at", x, y) end
        if input.mouse_released(mv.MOUSE_LEFT) then print("Left released at", x, y) end

        local controller = "Controller 0 disconnected"
        if input.pad_connected(0) then
            if input.pad_pressed(0, mv.PAD_A) then print("A pressed") end
            if input.pad_released(0, mv.PAD_A) then print("A released") end
            controller = ("Controller A held: %s  Stick x: %.2f"):format(
                tostring(input.pad_button(0, mv.PAD_A)), input.pad_axis(0, mv.PAD_LEFT_X))
        end

        local function label(text, y)
            mv.draw_text {text = text, x = 12, y = y, size = 16}
        end
        label("Space held: " .. tostring(input.key(mv.KEY_SPACE)), 12)
        label(("Pointer: %d, %d  Left held: %s"):format(
            x, y, tostring(input.mouse_button(mv.MOUSE_LEFT))), 36)
        label(controller, 60)
        label("Typed: " .. typed, 84)
        -- Tap the label to open the system keyboard in an exported Android game.
        if input.mouse_pressed(mv.MOUSE_LEFT) and y >= 84 and y < 116 then
            keyboard_open = not keyboard_open
            input.text_input_set_keyboard(keyboard_open)
        end
        mv.frame()
    end
end)

mv.start(function() show() end, {title = "Input monitor", width = 420, height = 116})
