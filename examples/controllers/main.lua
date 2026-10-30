-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

require("malvado").import_globals()
local names = {"a", "b", "x", "y", "up", "down", "left", "right", "lb", "rb", "back", "start"}
local axes = {"left_x", "left_y", "right_x", "right_y", "left_trigger", "right_trigger"}

local Monitor = process(function()
  while not key("escape") do
    draw_text("Connect or disconnect a controller. Escape exits.", 20, 20, 20)
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
        draw_text("Slot " .. slot .. ": connected", 20, y, 20)
        draw_text("Buttons: " .. table.concat(held, " "), 20, y + 25, 18)
        draw_text(table.concat(values, "  ", 1, 4), 20, y + 50, 18)
        draw_text(table.concat(values, "  ", 5, 6), 20, y + 75, 18)
      else
        draw_text("Slot " .. slot .. ": disconnected", 20, y, 20)
      end
    end
    frame()
  end
end)

start(function()
  clear_screen(25, 30, 45)
  Monitor()
end, {width = 800, height = 570, title = "Controller monitor"})
