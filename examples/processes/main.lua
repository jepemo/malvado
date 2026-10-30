-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

require("malvado").import_globals()
local child = process(function(self)
  while true do
    local message = self:recv()
    if message then print("Message:", message.text) end
    frame()
  end
end)
local parent = process(function()
  local id = child {}
  send(id, {text = "hello from the parent process"})
  frame()
  frame()
  kill(id)
end)
start(function() parent {} end)
