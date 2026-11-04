-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local defaults = {nested = {value = 12}}
local process = mv.process(function() error("process must not run") end, {defaults = defaults})
local weak = setmetatable({}, {__mode = "v"})
local payload = {}
local fields = {payload = payload}
weak[1] = payload

test_arm()
local ok, handle = pcall(process, fields)
test_disarm()
assert(fields.payload == payload and defaults.nested.value == 12)
fields, payload = nil, nil
if not ok then
  collectgarbage("collect")
  collectgarbage("collect")
  assert(weak[1] == nil, "failed spawn retained input")
  assert(#mv.snapshot().processes == 0, "failed spawn left a process")
  handle = process()
end
assert(handle.id == 1, "failed spawn consumed a process ID")
assert(mv.kill(handle))
mv.start(function() end)
assert(#mv.snapshot().processes == 0)
test_report(ok)
