-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local actor = mv.process(function()
  local id = mv.write(mv.font(20), 1, 2, "Lua", "center")
  mv.frame()
  mv.change_text(id, "Changed") -- Preserve the coordinates/alignment set by C.
  mv.frame()
  assert(not pcall(mv.change_text, id, "removed"))
  assert(not pcall(mv.delete_text, id))
  collectgarbage("collect")
end)
mv.start(function() actor() end)
