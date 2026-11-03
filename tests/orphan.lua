-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local parent_id, child_id, other_id
local child = mv.process(function(self)
  assert(self.parent == nil and self.creator_id == parent_id.id)
  while true do mv.frame() end
end)
local parent = mv.process(function(self)
  parent_id = self
  child_id = mv.process.spawn(child, {}, {attached = false})
end)
local other = mv.process(function() while true do mv.frame() end end)
local controller = mv.process(function()
  mv.frame()
  assert(mv.status(parent_id) == nil)
  assert(mv.status(child_id) == "running")
  mv.kill_tree(other_id)
  assert(mv.status(child_id) == "running")
  assert(not pcall(mv.signal_tree, parent_id, "freeze", {relation = "ownership"}))
  mv.signal_tree(parent_id, "freeze")
  assert(mv.status(child_id) == "frozen")
  mv.signal_tree(parent_id, "wakeup")
  assert(mv.status(child_id) == "running")
  mv.kill_tree(parent_id)
  assert(mv.status(child_id) == nil)
  mv.frame()
end)
mv.start(function()
  controller {}
  parent {}
  other_id = other {}
end)
