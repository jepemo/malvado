-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local actor = mv.process(function()
  local id = mv.write {font = mv.font {size = 20}, text = "Lua", x = 1, y = 2, align = "center"}
  mv.frame()
  mv.change_text(id, {text = "Changed"}) -- Preserve the coordinates/alignment set by C.
  mv.frame()
  assert(not pcall(mv.change_text, id, {text = "removed"}))
  assert(not pcall(mv.delete_text, id))
  collectgarbage("collect")
end)
mv.start(function() actor() end)
