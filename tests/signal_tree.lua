-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local root_id, child_id, unrelated_id
local root_ticks, child_ticks, unrelated_ticks = 0, 0, 0
local child = mv.process(function()
  while true do child_ticks = child_ticks + 1; mv.frame() end
end)
local root = mv.process(function()
  child_id = child {}
  while true do root_ticks = root_ticks + 1; mv.frame() end
end)
local unrelated = mv.process(function()
  while true do unrelated_ticks = unrelated_ticks + 1; mv.frame() end
end)
local controller = mv.process(function()
  mv.frame()
  mv.frame()
  assert(child_id and mv.status(child_id) == "running")
  mv.signal_tree(root_id, "sleep")
  local r, c, u = root_ticks, child_ticks, unrelated_ticks
  assert(mv.status(root_id) == "sleeping" and mv.status(child_id) == "sleeping")
  mv.frame()
  assert(root_ticks == r and child_ticks == c and unrelated_ticks > u)
  mv.signal_tree(root_id, "wakeup")
  mv.frame()
  assert(root_ticks > r and child_ticks > c)
  mv.signal_tree(root_id, "freeze")
  assert(mv.status(root_id) == "frozen" and mv.status(child_id) == "frozen")
  assert(not pcall(mv.signal_tree, root_id, "invalid"))
  assert(mv.status(root_id) == "frozen" and mv.status(child_id) == "frozen")
  mv.kill(child_id)
  mv.signal_tree(root_id, "wakeup")
  assert(mv.status(child_id) == nil)
  mv.kill_tree(root_id)
  mv.kill(unrelated_id)
  mv.frame()
end)
mv.start(function()
  controller {}
  root_id = root {}
  unrelated_id = unrelated {}
end)
