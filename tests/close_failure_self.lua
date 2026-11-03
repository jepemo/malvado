-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local proc = require("malvado").process
local good = proc(function()
  local guard <close> = setmetatable({}, {__close = function() print("CLOSE_GOOD") end})
  while true do proc.frame() end
end)
local bad = proc(function(self)
  local first <close> = setmetatable({}, {__close = function() print("CLOSE_PARENT") end})
  local second <close> = setmetatable({}, {__close = function()
    print("CLOSE_BAD")
    error("close after self-kill")
  end})
  proc.kill(self)
  error("self-kill returned")
end)
require("malvado").start(function() good {}; bad {} end)
