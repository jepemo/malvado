-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local parent_id, child_id, other_id
local child = mv.process(function(self)
  assert(self.parent == parent_id)
  while true do mv.frame() end
end)
local parent = mv.process(function(self)
  parent_id = self.id
  child_id = child {}
end)
local other = mv.process(function() while true do mv.frame() end end)
local controller = mv.process(function()
  mv.frame()
  assert(mv.status(parent_id) == nil)
  assert(mv.status(child_id) == "running")
  mv.kill_tree(other_id)
  assert(mv.status(child_id) == "running")
  mv.kill_tree(child_id)
  mv.frame()
end)
mv.start(function()
  controller {}
  parent {}
  other_id = other {}
end)
