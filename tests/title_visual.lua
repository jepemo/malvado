-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local task = mv.process(function()
  while not mv.key("escape") do mv.frame() end
end)
mv.start(function()
  assert(not pcall(mv.set_title, "bad\0title"))
  mv.set_title("Updated title")
  task()
end, {title = "Initial title"})
