-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local closed, after, saved = 0, false, nil
local P = mv.process(function(self)
  saved = self
  local resource <close> = setmetatable({}, {__close = function() closed = closed + 1 end})
  pcall(function()
    mv.kill(self)
    after = true
  end)
  after = true
end)
local tree_closed, tree_after, rejected = 0, false, false
local root_id
local child = mv.process(function(self)
  local resource <close> = setmetatable({}, {__close = function() tree_closed = tree_closed + 1 end})
  local value = setmetatable({}, {__tostring = function()
    local ok = pcall(mv.kill_tree, root_id)
    rejected = not ok and mv.status(root_id) == "running" and mv.status(self.id) == "running"
    return "checked"
  end})
  tostring(value)
  pcall(mv.kill_tree, root_id)
  tree_after = true
end)
local root = mv.process(function()
  child {}
  while true do mv.frame() end
end)
mv.start(function() P {}; root_id = root {} end)
assert(closed == 1 and not after and saved.reason == "killed")
assert(rejected and tree_closed == 1 and not tree_after)
