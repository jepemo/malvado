-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

require("malvado").import_globals()
local enemy = process(function(self)
  while true do
    draw_text("Enemy " .. self.id, 20, 20, 20)
    frame()
  end
end)
local controller = process(function()
  local id = enemy {}
  frame()
  frame()
  local state = snapshot()
  print("Tick:", state.tick, "Enemies:", #find(enemy))
  kill(id)
end)
start(function() controller {} end)
