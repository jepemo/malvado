-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

require("malvado").import_globals()
local names = {"a", "b", "x", "y", "up", "down", "left", "right", "lb", "rb", "back", "start"}
local axes = {"left_x", "left_y", "right_x", "right_y", "left_trigger", "right_trigger"}

local Monitor = process(function()
  while not key("escape") do
    draw_text {text = "Connect or disconnect a controller. Escape exits.", x = 20, y = 20, size = 20}
    for slot = 0, 3 do
      local y = 60 + slot * 120
      if pad_connected(slot) then
        local held = {}
        for _, name in ipairs(names) do
          if pad_button(slot, name) then held[#held + 1] = name end
        end
        local values = {}
        for _, axis in ipairs(axes) do
          values[#values + 1] = string.format("%s %.2f", axis, pad_axis(slot, axis))
        end
        draw_text {text = "Slot " .. slot .. ": connected", x = 20, y = y, size = 20}
        draw_text {text = "Buttons: " .. table.concat(held, " "), x = 20, y = y + 25, size = 18}
        draw_text {text = table.concat(values, "  ", 1, 4), x = 20, y = y + 50, size = 18}
        draw_text {text = table.concat(values, "  ", 5, 6), x = 20, y = y + 75, size = 18}
      else
        draw_text {text = "Slot " .. slot .. ": disconnected", x = 20, y = y, size = 20}
      end
    end
    frame()
  end
end)

start(function()
  clear_screen {25, 30, 45, 255}
  Monitor()
end, {width = 800, height = 570, title = "Controller monitor"})
