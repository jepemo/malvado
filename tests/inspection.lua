-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local a, b
local weak = setmetatable({}, {__mode = "v"})
local worker = mv.process(function(self)
  while true do
    mv.draw_text {text = "hello", x = 0, y = 0, size = 20}
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
  a.payload = {}
  weak[1] = a.payload
  local snap = mv.snapshot()
  assert(snap.tick >= 2)
  assert(type(snap.runtime) == "string" and #snap.runtime == 48 and snap.runtime:match("^%x+$"))
  local record = lookup(snap, a)
  assert(record.id == a.id and record.parent == nil and record.creator_id == nil and not record.attached)
  assert(math.type(record.type) == "integer" and record.type == lookup(snap, b).type)
  assert(record.priority == a.priority and record.messages == 0)
  assert(record.status == mv.STATUS_RUNNING and record.visible == true)
  assert(#snap.lineage == #snap.processes)
  assert(lookup(snap, a).draw_commands == 1)
  assert(lookup(snap, b).draw_commands == 1)
  record.priority = 42
  snap.lineage[1].alive = false
  assert(a.priority == 0 and mv.snapshot().lineage[1].alive)
  mv.signal(a, "sleep")
  mv.signal(b, "freeze")
  local frozen = mv.snapshot()
  assert(frozen.runtime == snap.runtime)
  assert(not lookup(frozen, a).visible and lookup(frozen, b).visible)
  assert(lookup(frozen, b).status == "frozen")
  local payload = {}
  weak[2] = payload
  mv.send(a, payload)
  payload = nil
  mv.frame()
  local again = mv.snapshot()
  assert(lookup(again, a).draw_commands == 1 and lookup(again, b).draw_commands == 1)
  assert(lookup(again, a).priority == 0 and lookup(again, a).messages == 1)
  assert(lookup(snap, a).status == "running") -- Prior snapshots are copies.
  assert(mv.process.alive(a))
  assert(mv.kill(a) and not mv.signal(a, "kill") and not mv.kill(a) and not mv.process.alive(a))
  assert(mv.signal(b, "kill") and not mv.kill(b))
  assert(#mv.find(worker) == 0)
  mv.frame()
  collectgarbage("collect")
  collectgarbage("collect")
  assert(weak[1] == nil) -- The retained snapshot does not own process fields.
  assert(weak[2] == nil) -- It also does not retain messages.
  assert(lookup(snap, a).status == "running")
end)
mv.start(function()
  controller {}
  a = worker {}
  b = worker {}
end)
