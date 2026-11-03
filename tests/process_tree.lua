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
  local children = mv.process.children(ids.parent)
  local descendants = mv.process.descendants(ids.parent)
  assert(#children == 1 and children[1] == ids.child)
  assert(#descendants == 2 and descendants[1] == ids.child and descendants[2] == ids.grandchild)
  local filtered = mv.process.descendants(ids.parent, {type = grandchild})
  assert(#filtered == 1 and filtered[1] == ids.grandchild)
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
assert(pcall(mv.kill_tree, ids.parent))
