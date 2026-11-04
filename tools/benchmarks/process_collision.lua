-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local proc = mv.process
local count = 1000
local graph = mv.graphics.circle(2)
local Enemy = proc(function() while true do proc.frame() end end, {defaults = {graph = graph}})
local Controller = proc(function(self)
  self.graph = graph
  local enemies = {}
  for i = 1, count do enemies[i] = Enemy({x = 100 + i * 16}) end
  local start = os.clock()
  for i = 1, count do enemies[i].x = enemies[i].x + 1 end
  local updates = os.clock()
  for _ = 1, count do assert(proc.collision(self, Enemy) == nil) end
  local queries = os.clock()
  print(string.format("processes=%d queries=%d lua_updates_ms=%.3f lua_misses_ms=%.3f",
        count, count, (updates - start) * 1000, (queries - updates) * 1000))
end)
mv.start(function() Controller() end)
