-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local draw_text, frame = mv.draw_text, mv.frame
local ticks, seen = 0, {}
local interactive = os.getenv("MALVADO_REAL_INPUT") == "1"

mv.draw_text = function(text, ...)
  seen[text] = true
  return draw_text(text, ...)
end
if not interactive then
  local keys = {
    [0] = "enter", [2] = "escape", [4] = "down", [6] = "enter",
    [8] = "escape", [10] = "down", [12] = "enter", [14] = "escape",
    [16] = "down", [18] = "enter", [20] = "escape", [22] = "escape"
  }
  mv.key = function(key) return keys[ticks] == key end
  mv.frame = function() ticks = ticks + 1; frame() end
end

dofile("examples/demo/main.lua")
assert(seen["Sprites - Escape: menu"] and seen["Keyboard - Escape: menu"])
if not interactive then
  assert(seen["Texts - Escape: menu"] and seen["Fade - Escape: menu"])
end
