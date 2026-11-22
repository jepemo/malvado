-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local worker = mv.process(function()
  while true do
    mv.draw_text {text = "X", x = 30, y = 30, size = 40}
    mv.frame()
  end
end)
local controller = mv.process(function() while true do mv.frame() end end)
mv.start(function() worker {}; controller {} end, {width = 160, height = 120})
