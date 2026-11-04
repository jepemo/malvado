-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local closed = {}
local child = mv.process(function(self)
  local guard <close> = setmetatable({}, {__close = function()
    assert(self.status == "dead")
    closed[#closed + 1] = "child"
  end})
  mv.frame()
end)
local parent = mv.process(function(self)
  local guard <close> = setmetatable({}, {__close = function()
    assert(self.status == "dead")
    closed[#closed + 1] = "parent"
  end})
  child()
  while true do mv.frame() end
end)
local control = mv.process(function()
  local root = parent()
  mv.frame()
  mv.frame()
  mv.frame()
  mv.kill(root)
  assert(root.status == "dead")
  mv.frame()
  assert(#closed == 2 and closed[1] == "parent" and closed[2] == "child")
end)
mv.start(function() control() end)
