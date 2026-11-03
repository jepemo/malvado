-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local proc = require("malvado").process
local bad = proc(function()
  local guard <close> = setmetatable({}, {__close = function()
    print("CLOSE_BAD")
    error({reason = "not a string"})
  end})
  while true do proc.frame() end
end)
local good = proc(function()
  local guard <close> = setmetatable({}, {__close = function() print("CLOSE_GOOD") end})
  while true do proc.frame() end
end)
local parent = proc(function()
  local guard <close> = setmetatable({}, {__close = function() print("CLOSE_PARENT") end})
  bad {}
  good {}
  while true do proc.frame() end
end)
local controller = proc(function()
  local root = parent {}
  proc.frame()
  proc.frame()
  proc.frame()
  proc.kill(root)
  proc.frame()
end)
require("malvado").start(function() controller {} end)
