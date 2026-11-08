-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local proc = mv.process
local root, first, later, unrelated
local create_later = false
local worker = proc(function() while true do proc.frame() end end)
local owner = proc(function()
  first = worker {}
  while true do
    if create_later and not later then later = worker {} end
    proc.frame()
  end
end)
local controller = proc(function()
  proc.frame()
  proc.frame()
  local before = proc.descendants(root, {relation = "creation"})
  assert(#before == 1 and before[1] == first)
  proc.signal_tree(root, "freeze")
  assert(first.status == "frozen" and unrelated.status == "running")
  create_later = true
  proc.signal(root, "wakeup")
  proc.frame()
  assert(later and later.status == "running" and first.status == "frozen")
  assert(#before == 1 and before[1] == first)
  local after = proc.descendants(root, {relation = "creation"})
  assert(#after == 2 and after[1] == first and after[2] == later)
  proc.kill_tree(root)
  proc.kill(unrelated)
end)
mv.start(function() controller {}; root = owner {}; unrelated = worker {} end)
