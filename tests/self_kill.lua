-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local closed, after, saved, killed_thread = 0, false, nil, nil
local P = mv.process(function(self)
  saved = self
  killed_thread = coroutine.running()
  local value = setmetatable({}, {__tostring = function()
    local ok = pcall(mv.kill, self)
    assert(not ok and self.status == "running")
    return "checked"
  end})
  tostring(value)
  local resource <close> = setmetatable({}, {__close = function() closed = closed + 1 end})
  xpcall(function()
    mv.kill(self)
    after = true
  end, function() error("self-kill was caught") end)
  after = true
end)
local observer = mv.process(function()
  local ok, message = coroutine.resume(killed_thread)
  assert(not ok and message:find("managed process", 1, true))
  ok, message = coroutine.close(killed_thread)
  assert(not ok and message:find("managed process", 1, true))
end)
local tree_closed, tree_after, rejected = 0, false, false
local root_id
local child = mv.process(function(self)
  local resource <close> = setmetatable({}, {__close = function() tree_closed = tree_closed + 1 end})
  local value = setmetatable({}, {__tostring = function()
    local ok = pcall(mv.kill_tree, root_id)
    local owned_ok = pcall(mv.signal_tree, root_id, "kill", {relation = "ownership"})
    rejected = not ok and not owned_ok and mv.status(root_id) == "running" and mv.status(self) == "running"
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
local ordinary = coroutine.create(function() coroutine.yield(42); return 43 end)
local ok, value = coroutine.resume(ordinary)
assert(ok and value == 42)
ok, value = coroutine.resume(ordinary)
assert(ok and value == 43 and coroutine.close(ordinary))
local spare = coroutine.create(function() coroutine.yield() end)
assert(coroutine.resume(spare) and coroutine.close(spare))
mv.start(function() P {}; observer {}; root_id = root {} end)
assert(closed == 1 and not after and saved.reason == "killed")
assert(rejected and tree_closed == 1 and not tree_after)
