-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local worker = mv.process(function()
  while true do mv.frame() end
end)
mv.start(function() worker {} end)
