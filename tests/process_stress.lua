-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local closed, received = 0, 0
local child = mv.process(function(self)
  local resource <close> = setmetatable({}, {__close = function() closed = closed + 1 end})
  while true do
    local message = self:recv()
    if message == nil then break end
    received = received + 1
    assert(message == received)
  end
  mv.frame()
end)
local parent = mv.process(function()
  local first
  for i = 1, 300 do
    local id = child {}
    if i == 1 then first = id end
  end
  for i = 1, 1024 do mv.send(first, i) end
  assert(not pcall(mv.send, first, 1025))
  collectgarbage("collect")
  mv.frame()
  mv.frame()
end)
mv.start(function() parent {} end)
assert(received == 1024 and closed == 300)
