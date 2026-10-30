-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

require("malvado").import_globals()
local worker = process(function(self)
  print("Child", self.id, "of process", self.parent)
  while true do frame() end
end)
local group = process(function(self)
  worker {}
  worker {}
  print("Group", self.id, "created")
  while true do frame() end
end)
local controller = process(function()
  local group_id = group {}
  frame()
  frame()
  frame()
  kill_tree(group_id)
  print("Group and descendants stopped")
end)
start(function() controller {} end)
