-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local child = mv.process(function(self)
  while true do
    local message = self:recv()
    if message then print("Mensaje:", message.text) end
    mv.frame()
  end
end)
local parent = mv.process(function()
  local id = child {}
  mv.send(id, {text = "hola desde el proceso padre"})
  mv.frame()
  mv.frame()
  mv.kill(id)
end)
mv.start(function() parent {} end)
