-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local native_type = mv.process(function() error("unexpected Lua process") end)
local saved
_G.guard = setmetatable({}, {__gc = function()
  local ok = pcall(function()
    assert(saved.id > 0 and saved.status == "dead" and saved.reason == "killed")
    assert(not pcall(function() return saved.payload end))
  end)
  if ok then print("native shutdown handle valid") end
end})
local controller = mv.process(function()
  mv.frame()
  local found = mv.find(native_type)
  assert(#found == 1)
  saved = found[1]
  assert(mv.find(native_type)[1] == saved)
  saved.payload = {}
  mv.frame()
  error("force native shutdown")
end)
mv.start(function() controller() end)
