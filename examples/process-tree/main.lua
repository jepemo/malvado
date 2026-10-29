-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local worker = mv.process(function(self)
  print("Hijo", self.id, "del proceso", self.parent)
  while true do mv.frame() end
end)
local group = mv.process(function(self)
  worker {}
  worker {}
  print("Grupo", self.id, "creado")
  while true do mv.frame() end
end)
local controller = mv.process(function()
  local group_id = group {}
  mv.frame()
  mv.frame()
  mv.frame()
  mv.kill_tree(group_id)
  print("Grupo y descendientes detenidos")
end)
mv.start(function() controller {} end)
