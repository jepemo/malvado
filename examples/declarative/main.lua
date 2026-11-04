-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local task = mv.process(function()
  print("Hello from a process")
end)

return {
  title = "Declarative game",
  width = 640,
  height = 480,
  init = function()
    task()
  end,
}
