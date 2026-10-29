-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local worker = mv.process(function(self)
  for i = 1, 5000 do
    mv.send(self.id, i)
    assert(self:recv() == i)
  end
  assert(self:recv() == nil)
end)
mv.start(function() worker {} end)
