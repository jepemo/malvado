-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local worker = mv.process(function(self)
  print("Child", self.id, "of process", self.parent)
  while true do mv.frame() end
end)
local group = mv.process(function(self)
  worker {}
  worker {}
  print("Group", self.id, "created")
  while true do mv.frame() end
end)
local controller = mv.process(function()
  local group_id = group {}
  mv.frame()
  mv.frame()
  mv.frame()
  mv.kill_tree(group_id)
  print("Group and descendants stopped")
end)
mv.start(function() controller {} end)
