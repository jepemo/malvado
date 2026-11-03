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
    if item.id == id.id then return item end
  end
end
local controller = mv.process(function()
  mv.frame()
  local all = mv.find(worker)
  assert(#all == 2 and all[1] == a and all[2] == b)
  assert(not pcall(mv.find, function() end))
  local invalid = a.id
  assert(not pcall(mv.status, invalid))
  assert(not pcall(mv.signal, invalid, "kill"))
  assert(not pcall(mv.kill, invalid))
  assert(not pcall(mv.signal_tree, invalid, "kill"))
  assert(not pcall(mv.kill_tree, invalid))
  assert(not pcall(mv.send, invalid, "message"))
  assert(not pcall(mv.set_priority, invalid, 1))
  assert(not pcall(mv.get_priority, invalid))
  assert(not pcall(mv.process.alive, invalid))
  assert(not pcall(mv.process.children, invalid))
  assert(not pcall(mv.process.descendants, invalid))
  assert(a.status == "running" and b.status == "running")
  local snap = mv.snapshot()
  assert(snap.tick >= 2)
  assert(type(snap.runtime) == "string" and #snap.runtime == 48 and snap.runtime:match("^%x+$"))
  assert(lookup(snap, a).creator_id == nil and not lookup(snap, a).attached)
  assert(#snap.lineage == #snap.processes)
  assert(lookup(snap, a).draw_commands == 1)
  assert(lookup(snap, b).draw_commands == 1)
  mv.signal(a, "sleep")
  mv.signal(b, "freeze")
  local frozen = mv.snapshot()
  assert(frozen.runtime == snap.runtime)
  assert(not lookup(frozen, a).visible and lookup(frozen, b).visible)
  assert(lookup(frozen, b).status == "frozen")
  mv.frame()
  local again = mv.snapshot()
  assert(lookup(again, a).draw_commands == 1 and lookup(again, b).draw_commands == 1)
  assert(lookup(snap, a).status == "running") -- Prior snapshots are copies.
  assert(mv.process.alive(a))
  assert(mv.kill(a) and not mv.signal(a, "kill") and not mv.kill(a) and not mv.process.alive(a))
  assert(mv.signal(b, "kill") and not mv.kill(b))
  assert(#mv.find(worker) == 0)
  mv.frame()
end)
mv.start(function()
  controller {}
  a = worker {}
  b = worker {}
end)
