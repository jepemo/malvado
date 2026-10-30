-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local smoke = os.getenv("MALVADO_SMOKE") == "1"
local names = {"a", "b", "x", "y", "up", "down", "left", "right", "lb", "rb", "back", "start"}
local axes = {"left_x", "left_y", "right_x", "right_y", "left_trigger", "right_trigger"}

local Monitor = mv.process(function()
  local ticks = 0
  while not mv.key("escape") do
    ticks = ticks + 1
    mv.draw_text("Connect or disconnect a controller. Escape exits.", 20, 20, 20)
    for slot = 0, 3 do
      local y = 60 + slot * 120
      if mv.pad_connected(slot) then
        local held = {}
        for _, name in ipairs(names) do
          if mv.pad_button(slot, name) then held[#held + 1] = name end
        end
        local values = {}
        for _, axis in ipairs(axes) do
          values[#values + 1] = string.format("%s %.2f", axis, mv.pad_axis(slot, axis))
        end
        mv.draw_text("Slot " .. slot .. ": connected", 20, y, 20)
        mv.draw_text("Buttons: " .. table.concat(held, " "), 20, y + 25, 18)
        mv.draw_text(table.concat(values, "  ", 1, 4), 20, y + 50, 18)
        mv.draw_text(table.concat(values, "  ", 5, 6), 20, y + 75, 18)
      else
        mv.draw_text("Slot " .. slot .. ": disconnected", 20, y, 20)
      end
    end
    if smoke and ticks == 5 then
      print("controller monitor smoke: four slots")
      return
    end
    mv.frame()
  end
end)

mv.start(function()
  mv.clear_screen(25, 30, 45)
  Monitor()
end, {width = 800, height = 570, title = "Controller monitor"})
