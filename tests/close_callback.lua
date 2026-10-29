-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local count = 0
local child = mv.process(function() count = count + 1 end)
local owner = mv.process(function()
  local guard <close> = setmetatable({}, {__close = function()
    for _ = 1, 100 do child {} end
  end})
  mv.frame()
end)
mv.start(function() owner {} end)
assert(count == 100)
