-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local closed, after = 0, false
local P = mv.process(function(self)
  local resource <close> = setmetatable({}, {__close = function() closed = closed + 1 end})
  mv.kill(self.id)
  mv.frame()
  after = true
end)
mv.start(function() P {} end)
assert(closed == 1 and not after)
