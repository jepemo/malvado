-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local root_id
local checked = false
local child
child = mv.process(function(self)
  assert(self.creator_id == root_id.id)
  local history = mv.process.descendants(root_id, {relation = "creation", type = child})
  assert(#history == 1 and history[1] == self)
  local children = mv.process.children(root_id, {relation = "creation"})
  assert(#children == 1 and children[1] == self)
  assert(not pcall(mv.process.descendants, root_id, {relation = "invalid"}))
  assert(not pcall(mv.process.descendants, root_id, {relation = "creation", type = 1}))
  assert(not pcall(function() self.creator_id = 99 end))
  local snapshot = mv.snapshot()
  local root, own
  for _, node in ipairs(snapshot.lineage) do
    if node.id == root_id.id then root = node end
    if node.id == self.id then own = node end
  end
  assert(root and not root.alive and root.creator_id == nil)
  assert(own and own.alive and own.creator_id == root_id.id)
  checked = true
end)
local root = mv.process(function() mv.process.spawn(child, {}, {attached = false}) end)
mv.start(function() root_id = root {} end)
assert(checked)
