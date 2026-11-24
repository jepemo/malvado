-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local task = mv.process(function(self) assert(self.fps == 0) end)
assert(task().fps == 0)

return {init = function()
  assert(task().fps == 0)
  mv.set_fps(15)
end}
