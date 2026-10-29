-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local a, b
local worker = mv.process(function(self)
  while true do
    mv.draw_text("hello", 0, 0, 20)
    mv.frame()
  end
end)
local function lookup(snapshot, id)
  for _, item in ipairs(snapshot.processes) do
    if item.id == id then return item end
  end
end
local controller = mv.process(function()
  mv.frame()
  local all = mv.find(worker)
  assert(#all == 2 and all[1] == a and all[2] == b)
  assert(not pcall(mv.find, function() end))
  local snap = mv.snapshot()
  assert(snap.tick >= 2)
  assert(lookup(snap, a).draw_commands == 1)
  assert(lookup(snap, b).draw_commands == 1)
  mv.signal(a, "sleep")
  mv.signal(b, "freeze")
  local frozen = mv.snapshot()
  assert(not lookup(frozen, a).visible and lookup(frozen, b).visible)
  assert(lookup(frozen, b).status == "frozen")
  mv.frame()
  local again = mv.snapshot()
  assert(lookup(again, a).draw_commands == 1 and lookup(again, b).draw_commands == 1)
  assert(lookup(snap, a).status == "running") -- Prior snapshots are copies.
  mv.kill(a); mv.kill(b)
  assert(#mv.find(worker) == 0)
  mv.frame()
end)
mv.start(function()
  controller {}
  a = worker {}
  b = worker {}
end)
