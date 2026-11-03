-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
mv.import_globals()
for _, name in ipairs({"sleep", "freeze", "wakeup", "kill"}) do
  local constant = "SIGNAL_" .. name:upper()
  assert(mv.process[constant] == name and _G[constant] == name)
end
local events, target_id = {}, nil
local closed = 0
local shared = { value = 1 }
local target = mv.process(function(self)
  local cleanup <close> = setmetatable({}, { __close = function() closed = closed + 1 end })
  events[#events + 1] = "target:start"
  local message = self:recv()
  assert(message == shared and message.value == 2)
  assert(self:recv() == nil)
  mv.frame()
  events[#events + 1] = "target:wake"
  mv.frame()
  error("killed process ran again")
end)
local controller = mv.process(function()
  mv.signal(target_id, SIGNAL_SLEEP)
  assert(mv.status(target_id) == "sleeping")
  mv.send(target_id, shared)
  shared.value = 2
  mv.frame()
  mv.signal(target_id, SIGNAL_WAKEUP)
  mv.frame()
  mv.signal(target_id, SIGNAL_FREEZE)
  assert(mv.status(target_id) == "frozen")
  mv.frame()
  mv.signal(target_id, SIGNAL_WAKEUP)
  mv.frame()
  mv.signal(target_id, SIGNAL_KILL)
  assert(mv.status(target_id) == nil)
  assert(not pcall(mv.send, target_id, {}))
  mv.frame()
end)
mv.start(function()
  controller {}
  target_id = target {}
end)
assert(table.concat(events, ",") == "target:start,target:wake")
assert(closed == 1)
assert(not pcall(mv.signal, target_id, "wakeup"))
