-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local weak = setmetatable({}, {__mode = "v"})
local receiver = mv.process(function()
  while true do mv.frame() end
end)
local controller = mv.process(function()
  local id = receiver {}
  local message = {contents = "pending"}
  weak[1] = message
  mv.send(id, message)
  message = nil
  collectgarbage("collect")
  assert(weak[1] ~= nil, "queued messages must remain alive")
  mv.kill(id)
  mv.frame()
  collectgarbage("collect")
  collectgarbage("collect")
  assert(weak[1] == nil, "killing a process must release queued messages")
end)
mv.start(function() controller {} end)
