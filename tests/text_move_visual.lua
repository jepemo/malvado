-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local task = mv.process(function()
  local text = mv.write(mv.font(24, 255, 0, 0), 10, 10, "Text")
  for _ = 1, 15 do mv.frame() end
  mv.change_text(text, "Moved", 100, 40, mv.font(24, 0, 255, 0))
  for _ = 1, 15 do mv.frame() end
  mv.delete_text(text)
end)
mv.start(function() task() end, {width = 240, height = 100, title = "Text move test"})
