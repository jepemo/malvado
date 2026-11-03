-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local shared = {value = 10}
local defaults = {health = 100, stats = shared, other = shared}
defaults.cycle = defaults
local seen = {}
local enemy = mv.process.define(function(self)
  assert(self.stats == self.other and self.cycle.stats == self.stats)
  seen[#seen + 1] = {self.health, self.stats.value}
  self.stats.value = self.health
end, {defaults = defaults})
local invalid = mv.process(function() error("must not run") end,
  {defaults = {nested = setmetatable({}, {})}})
local bad_fps = mv.process(function() error("must not run") end,
  {defaults = {fps = 61}})
mv.start(function()
  assert(not pcall(invalid))
  assert(not pcall(bad_fps))
  enemy {}
  enemy {health = 250}
end)
assert(#seen == 2 and seen[1][1] == 100 and seen[2][1] == 250)
assert(seen[1][2] == 10 and seen[2][2] == 10 and shared.value == 10)
