-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local proc = require("malvado").process
local forbidden = proc(function() error("shutdown process ran") end)
local good = proc(function()
  local guard <close> = setmetatable({}, {__close = function()
    assert(not pcall(proc.spawn, forbidden, {}, {attached = false}))
    print("CLOSE_GOOD")
  end})
  while true do proc.frame() end
end)
local bad = proc(function()
  local first <close> = setmetatable({}, {__close = function() print("CLOSE_PARENT") end})
  local second <close> = setmetatable({}, {__close = function(_, reason)
    assert(type(reason) == "string" and reason:find("process failed", 1, true))
    print("CLOSE_BAD")
    error("close after error")
  end})
  error("process failed")
end)
require("malvado").start(function() good {}; bad {} end)
