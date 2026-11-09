-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local process = mv.process
local owned, independent, root

local worker = process.define(function()
  while true do process.frame() end
end)

local group = process(function()
  owned = worker {}
  independent = process.spawn(worker, {}, {attached = false})
  while true do process.frame() end
end)

local controller = process(function()
  process.frame()
  assert(#process.children(root) == 1)
  assert(#process.descendants(root, {relation = "creation"}) == 2)
  assert(#process.find(worker) == 2)
  assert(#process.snapshot().processes == 4)

  process.set_priority(owned, 10)
  assert(process.get_priority(owned) == 10)
  process.signal(owned, "sleep")
  assert(process.status(owned) == "sleeping" and process.alive(owned))
  process.signal_tree(root, "freeze", {relation = "ownership"})
  assert(process.status(independent) == "running")
  process.signal_tree(root, "wakeup")
  process.kill_tree(root)
  assert(not process.alive(owned) and not process.alive(independent))
  assert(process.status(root) == nil and root.reason == "killed")

  local extra = worker {}
  assert(process.kill(extra) and not process.kill(extra))
  print("Process control example complete")
end)

mv.start(function()
  root = group {}
  controller {}
end)
