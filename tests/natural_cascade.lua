-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local proc = mv.process
local attached, independent, owner
local attached_calls, independent_calls = 0, 0
local linked = proc(function() attached_calls = attached_calls + 1 end)
local free = proc(function() independent_calls = independent_calls + 1 end)
local parent = proc(function()
  attached = linked {}
  independent = proc.spawn(free, {}, {attached = false})
end)
local controller = proc(function()
  owner = parent {}
  proc.frame()
  proc.frame()
  assert(owner.status == "dead" and owner.reason == "returned")
  assert(attached.status == "dead" and attached.reason == "killed")
  assert(independent.status == "running" and independent.parent == nil)
  local live = proc.descendants(owner, {relation = "creation"})
  assert(#live == 1 and live[1] == independent)
  proc.frame()
end)
mv.start(function() controller {} end)
assert(attached_calls == 0 and independent_calls == 1)
