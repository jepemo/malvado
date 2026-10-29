-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local ids, events = {}, {}
local closed = 0
local grandchild = mv.process(function(self)
  assert(self.parent == ids.child)
  local guard <close> = setmetatable({}, {__close = function() closed = closed + 1 end})
  events[#events + 1] = "grandchild"
  while true do mv.frame() end
end)
local child = mv.process(function(self)
  assert(self.parent == ids.parent)
  local guard <close> = setmetatable({}, {__close = function() closed = closed + 1 end})
  ids.grandchild = grandchild {}
  while true do mv.frame() end
end)
local parent = mv.process(function()
  local guard <close> = setmetatable({}, {__close = function() closed = closed + 1 end})
  ids.child = child {}
  while true do mv.frame() end
end)
local controller = mv.process(function()
  mv.frame()
  mv.frame()
  mv.frame()
  assert(mv.status(ids.grandchild) == "running")
  mv.kill_tree(ids.parent)
  assert(mv.status(ids.parent) == nil)
  assert(mv.status(ids.child) == nil)
  assert(mv.status(ids.grandchild) == nil)
  mv.frame()
end)
mv.start(function()
  controller {}
  ids.parent = parent {}
end)
assert(table.concat(events) == "grandchild")
assert(closed == 3)
assert(not pcall(mv.kill_tree, ids.parent))
