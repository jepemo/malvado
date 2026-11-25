-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
mv.import_globals()
assert(recv == mv.recv and recv == mv.process.recv)
assert(send == mv.send and send == mv.process.send)

local function fails(fragment, fn, ...)
  local ok, message = pcall(fn, ...)
  assert(not ok and tostring(message):find(fragment, 1, true), tostring(message))
end

local function check_empty(...)
  assert(select("#", ...) == 1 and ... == nil)
end

fails("running process", recv)
local parent_id, child_id
local payload = {}
local pending = setmetatable({}, {__mode = "v"})
local child = mv.process(function(self)
  assert(self.creator_id == parent_id)
  assert(self:recv() == "first")
  assert(recv() == "second\0part")
  local message = mv.process.recv()
  assert(message ~= nil and message == false)
  local integer = recv()
  assert(integer == 42 and math.type(integer) == "integer")
  assert(recv() == 1.5)
  assert(rawequal(recv(), payload))
  check_empty(recv())
  check_empty(recv(self))
  local unread = {}
  pending[1] = unread
  mv.send(self, unread)
end)
local parent = mv.process(function(self)
  parent_id = self.id
  child_id = child {}
  check_empty(mv.recv(child_id))
  mv.send(self, "parent")
  mv.send(child_id, "inspect child")
  mv.send(child_id, "inspect from coroutine")
  mv.send(child_id, "first")
  mv.send(child_id, "second\0part")
  mv.send(child_id, false)
  mv.send(child_id, 42)
  mv.send(child_id, 1.5)
  mv.send(child_id, payload)
  assert(not pcall(mv.send, child_id, nil))
  assert(not pcall(mv.signal, child_id, "unknown"))
  fails("malvado.process", recv, nil)
  fails("malvado.process", recv, self.id)
  fails("expected recv", recv, child_id, nil)
  assert(mv.process.recv(child_id) == "inspect child")
  local ok, message = coroutine.resume(coroutine.create(function()
    fails("running process", recv)
    assert(recv(child_id) == "inspect from coroutine")
  end))
  assert(ok, message)
  assert(recv() == "parent")
  check_empty(recv())

  local full = child {}
  for i = 1, 1024 do
    send(full, i)
  end
  assert(not pcall(send, full, "overflow"))
  for i = 1, 1024 do
    assert(mv.process.recv(full) == i)
  end
  check_empty(full:recv())
  assert(mv.kill(full))

  mv.frame()
  mv.frame()
  assert(mv.status(child_id) == mv.STATUS_DEAD)
  collectgarbage("collect")
  assert(pending[1] == nil)

  -- A nested coroutine can explicitly kill its caller before the caller returns.
  mv.send(self, "last message")
  ok, message = coroutine.resume(coroutine.create(function() assert(mv.kill(self)) end))
  assert(ok, message)
  fails("no longer alive", recv)
end)
mv.start(function()
  local handle = parent {}
  mv.send(handle, "setup")
  fails("running process", recv)
  assert(mv.process.recv(handle) == "setup")
end)
assert(mv.status(child_id) == mv.STATUS_DEAD)
fails("no longer alive", recv, child_id)
fails("running process", recv)
