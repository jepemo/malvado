-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

require("malvado").import_globals()
local names = {PAD_A, PAD_B, PAD_X, PAD_Y, PAD_UP, PAD_DOWN, PAD_LEFT, PAD_RIGHT, PAD_LB, PAD_RB, PAD_BACK, PAD_START}
local axes = {PAD_LEFT_X, PAD_LEFT_Y, PAD_RIGHT_X, PAD_RIGHT_Y, PAD_LEFT_TRIGGER, PAD_RIGHT_TRIGGER}

local Monitor = process(function()
  local last_change = {}
  while not key(KEY_ESCAPE) do
    draw_text {text = "Connect or disconnect a controller. Escape exits.", x = 20, y = 20, size = 20}
    for slot = 0, 3 do
      local y = 60 + slot * 120
      if pad_connected(slot) then
        local held, changes = {}, {}
        for _, name in ipairs(names) do
          if pad_button(slot, name) then held[#held + 1] = name end
          if pad_pressed(slot, name) then changes[#changes + 1] = "+" .. name end
          if pad_released(slot, name) then changes[#changes + 1] = "-" .. name end
        end
        if #changes > 0 then last_change[slot] = table.concat(changes, " ") end
        local values = {}
        for _, axis in ipairs(axes) do
          values[#values + 1] = string.format("%s %.2f", axis, pad_axis(slot, axis))
        end
        draw_text {text = "Slot " .. slot .. ": connected", x = 20, y = y, size = 20}
        draw_text {text = "Buttons: " .. table.concat(held, " "), x = 20, y = y + 25, size = 18}
        draw_text {text = table.concat(values, "  ", 1, 4), x = 20, y = y + 50, size = 18}
        draw_text {text = table.concat(values, "  ", 5, 6), x = 20, y = y + 75, size = 18}
        if last_change[slot] then
          draw_text {text = "Last change: " .. last_change[slot], x = 20, y = y + 95, size = 18}
        end
      else
        last_change[slot] = nil
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
