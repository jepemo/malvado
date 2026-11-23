-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local parent_id, child_id
local child = mv.process(function(self)
  assert(self.creator_id == parent_id)
  assert(self.recv == nil)
  assert(mv.recv(self) == "first")
  assert(mv.recv(self) == "second")
  assert(mv.recv(self) == nil and select("#", mv.recv(self)) == 1)
end)
local parent = mv.process(function(self)
  parent_id = self.id
  child_id = child {}
  assert(mv.recv(child_id) == nil)
  mv.send(child_id, "first")
  mv.send(child_id, "second")
  assert(not pcall(mv.send, child_id, nil))
  assert(not pcall(mv.signal, child_id, "unknown"))
  mv.frame()
end)
mv.start(function() parent {} end)
assert(mv.status(child_id) == nil)
assert(not pcall(mv.recv, child_id))
assert(not pcall(mv.recv, child_id.id))
