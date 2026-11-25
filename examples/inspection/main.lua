-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

require("malvado").import_globals()
local enemy = process(function(self)
  while true do
    draw_text {text = "Enemy " .. self.id, x = 20, y = 20, size = 20}
    frame()
  end
end)
local controller = process(function()
  local id = enemy {}
  frame()
  frame()
  local state = snapshot()
  print("Tick:", state.tick, "Enemies:", #find(enemy))
  print("Runtime:", state.runtime, "Lineage nodes:", #state.lineage)
  kill(id)
  assert(status(id) == STATUS_DEAD)
end)
start(function() controller {} end)
