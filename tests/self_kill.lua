-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
mv.import_globals()
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

local implicit_handle, queued = nil, setmetatable({}, {__mode = "v"})
local implicit_closed, owned_text = 0, nil
local implicit = mv.process(function(self)
  local nested = coroutine.create(function() mv.kill() end)
  local resumed, message = coroutine.resume(nested)
  assert(not resumed and message:find("requires a running process", 1, true))
  assert(not pcall(kill, nil) and not pcall(kill, self.id))
  assert(not pcall(kill, self, nil) and self.status == "running")
  local value = setmetatable({}, {__tostring = function()
    local killed, failure = pcall(kill)
    assert(not killed and failure:find("non-yieldable context", 1, true))
    assert(self.status == "running")
    return "checked"
  end})
  tostring(value)

  mv.draw_rect {x = 0, y = 0, width = 5, height = 5}
  local draw_count
  for _, item in ipairs(mv.snapshot().processes) do
    if item.id == self.id then draw_count = item.draw_commands end
  end
  assert(draw_count == 1)
  owned_text = mv.write {font = mv.font {size = 16}, text = "owned", x = 0, y = 0}
  local guard <close> = setmetatable({}, {__close = function()
    implicit_closed = implicit_closed + 1
  end})
  xpcall(function()
    pcall(function()
      kill()
      after = true
    end)
    after = true
  end, function() after = true end)
  after = true
end)
local check_cleanup = mv.process(function()
  mv.frame()
  assert(implicit_handle.reason == "killed" and not mv.kill(implicit_handle))
  assert(owned_text and not pcall(mv.change_text, owned_text, {text = "gone"}))
  for _, item in ipairs(mv.snapshot().processes) do
    assert(item.id ~= implicit_handle.id)
  end
  collectgarbage("collect")
  collectgarbage("collect")
  assert(queued[1] == nil, "self-kill must release queued messages")
end)

mv.start(function()
  local killed, failure = pcall(mv.kill)
  assert(not killed and failure:find("requires a running process", 1, true))
  P {}; observer {}; root_id = root {}
  implicit_handle = implicit {}
  local message = {damage = 50}
  queued[1] = message
  mv.send(implicit_handle, message)
  check_cleanup {}
end)
assert(closed == 1 and not after and saved.reason == "killed")
assert(implicit_closed == 1 and implicit_handle.reason == "killed")
assert(rejected and tree_closed == 1 and not tree_after)
