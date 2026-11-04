-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local task = mv.process(function()
  while not mv.key("escape") do mv.frame() end
end)
return {title = "Declarative initial title", init = function() task() end}
