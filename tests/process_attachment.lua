-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local proc = mv.process
local attached, independent, from_close, owner
local ran = 0
local worker = proc(function() ran = ran + 1 end)
local parent = proc(function(self)
  attached = worker {}
  independent = proc.spawn(worker, {}, {attached = false})
  local guard <close> = setmetatable({}, {__close = function()
    assert(not pcall(worker))
    from_close = proc.spawn(worker, {}, {attached = false})
  end})
  proc.kill(self)
  error("self-kill returned")
end)
local policy_attached, policy_free
local policy = proc(function(self)
  assert(self.children_attached)
  policy_attached = worker {}
  assert(policy_attached.parent == self)
  self.children_attached = false
  assert(not pcall(function() self.children_attached = "false" end))
  policy_free = worker {}
  assert(policy_free.parent == nil and policy_free.creator_id == self.id)
  proc.kill(self)
end, {children_attached = false})
local killer_ran_after = false
local killer = proc(function(self)
  proc.kill(self.parent)
  killer_ran_after = true
end)
local killer_parent = proc(function() killer {} while true do proc.frame() end end)
local controller = proc(function()
  local killer_owner = killer_parent {}
  local configured = policy {children_attached = true}
  owner = parent {}
  proc.frame()
  proc.frame()
  assert(configured.status == "dead" and policy_attached.status == "dead")
  assert(policy_free.status == "running")
  assert(owner.status == "dead" and attached.status == "dead")
  assert(independent.parent == nil and independent.creator_id == owner.id)
  assert(from_close.parent == nil and from_close.creator_id == owner.id)
  local descendants = proc.descendants(owner, {relation = "creation"})
  assert(#descendants == 2 and descendants[1] == independent and descendants[2] == from_close)
  proc.frame()
  assert(killer_owner.status == "dead" and not killer_ran_after)
end)
mv.start(function() controller {} end)
assert(ran == 3)
