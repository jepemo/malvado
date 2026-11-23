-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local worker = mv.process(function(self)
  for i = 1, 5000 do
    mv.send(self, i)
    assert(mv.recv(self) == i)
  end
  for i = 1, 1024 do mv.send(self, i) end
  assert(not pcall(mv.send, self, 1025))
  for i = 1, 1024 do assert(mv.recv() == i) end
  assert(mv.recv() == nil)
end)
mv.start(function() worker {} end)
