-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local id
local task = mv.process(function()
  assert(mv.font(24)[1] == 255)
  local f = mv.font(24, 255, 32, 12, 200)
  id = mv.write(f, 4, 5, "Hola")
  mv.frame()
  mv.change_text(id, "Adiós")
  mv.delete_text(id)
  assert(not pcall(mv.delete_text, id))
  assert(not pcall(mv.change_text, id, "X"))
end)
mv.start(function() task {} end)
assert(id ~= nil)
