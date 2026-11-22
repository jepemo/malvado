-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

require("malvado").import_globals()

local MIN_ALPHA = 125
local MAX_ALPHA = 255

-- One shared font styles three independent texts. Each text keeps its own
-- size and a copied color, so changing one never changes the others.
local BlipText = process(function(self)
  self.fps = 30

  local message = self.text or "Welcome 2 Malvado!"
  local width = self.width or get_screen_width()
  local height = self.height or get_screen_height()

  local f = font {size = 50}

  local colors = {
    {13, 59, 102, math.random(MIN_ALPHA, MAX_ALPHA)},
    {250, 240, 202, math.random(MIN_ALPHA, MAX_ALPHA)},
    {244, 211, 94, math.random(MIN_ALPHA, MAX_ALPHA)}
  }

  local texts = {}

  for i, color in ipairs(colors) do
    texts[i] = write {
      font = f,
      text = message,
      x = width / 2,
      y = height / 2,
      align = "center",
      size = 50 - (i - 1),
      color = color
    }
  end

  while true do
    for i, color in ipairs(colors) do
      color[4] = color[4] - 1

      if color[4] < MIN_ALPHA then
        color[4] = MAX_ALPHA
      end

      change_text(texts[i], {
        color = color
      })
    end

    frame()
  end
end)

return {
  title = "Blinking text",
  init = function()
    BlipText {}
  end,
}
