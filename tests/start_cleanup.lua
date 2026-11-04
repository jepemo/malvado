-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local task = mv.process(function()
  mv.frame()
  error("cleanup test failure")
end)
return {init = function() task() end}
