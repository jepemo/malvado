-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local enemy = mv.process(function(self)
  while true do
    mv.draw_text("Enemy " .. self.id, 20, 20, 20)
    mv.frame()
  end
end)
local controller = mv.process(function()
  local id = enemy {}
  mv.frame()
  mv.frame()
  local state = mv.snapshot()
  print("Tick:", state.tick, "Enemies:", #mv.find(enemy))
  mv.kill(id)
end)
mv.start(function() controller {} end)
