-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local finished, killed, live
_G.guard = setmetatable({}, {__gc = function()
  local ok = pcall(function()
    assert(finished.id > 0 and finished.status == "dead" and finished.reason == "returned")
    assert(killed.id > 0 and killed.status == "dead" and killed.reason == "killed")
    assert(live.id > 0 and live.status == "dead" and live.reason == "killed")
    assert(not pcall(function() return live.payload end))
  end)
  if ok then print("shutdown handles valid") end
end})
local done = mv.process(function() end)
local wait = mv.process(function(self)
  self.payload = {}
  while true do mv.frame() end
end)
local controller = mv.process(function()
  mv.frame()
  assert(finished.status == "dead" and finished.reason == "returned")
  mv.kill(killed)
  mv.frame()
  assert(killed.status == "dead" and killed.reason == "killed")
  error("force shutdown")
end)
mv.start(function()
  finished = done()
  killed = wait()
  live = wait()
  controller()
end)
