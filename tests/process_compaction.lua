-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local last, count = 0, 0
local ephemeral = mv.process(function(self)
  assert(self.id > last)
  last = self.id
  count = count + 1
end)
local controller = mv.process(function()
  for _ = 1, 300 do
    local id = ephemeral {}
    mv.frame()
    mv.frame()
    assert(mv.status(id) == nil)
  end
  local id = ephemeral {}
  assert(id > last)
  mv.frame()
end)
mv.start(function() controller {} end)
assert(count == 301)
