-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local proc = mv.process
local attached, independent, root
local worker = proc(function() while true do proc.frame() end end)
local owner = proc(function()
  attached = worker {}
  independent = proc.spawn(worker, {}, {attached = false})
  while true do proc.frame() end
end)
local controller = proc(function()
  proc.frame()
  proc.frame()
  local ownership = proc.descendants(root)
  local creation = proc.descendants(root, {relation = "creation"})
  assert(#ownership == 1 and ownership[1] == attached)
  assert(#creation == 2 and creation[1] == attached and creation[2] == independent)
  proc.signal_tree(root, "freeze", {relation = "ownership"})
  assert(root.status == "frozen" and attached.status == "frozen")
  assert(independent.status == "running")
  assert(not pcall(proc.signal_tree, root, "invalid", {relation = "creation"}))
  assert(root.status == "frozen" and attached.status == "frozen" and independent.status == "running")
  proc.signal_tree(root, "wakeup")
  proc.signal_tree(root, "freeze")
  assert(root.status == "frozen" and attached.status == "frozen" and independent.status == "frozen")
  local frozen = proc.descendants(root, {relation = "creation"})
  assert(#frozen == 2 and frozen[1] == attached and frozen[2] == independent)
  proc.kill_tree(root)
  assert(root.status == "dead" and attached.status == "dead" and independent.status == "dead")
  assert(creation[1] == attached and creation[1].status == "dead")
  assert(not pcall(function() return attached.x end))
  assert(not pcall(function() attached.x = 1 end))
end)
mv.start(function() controller {}; root = owner {} end)
