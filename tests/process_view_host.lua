-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local scroll = mv.graphics.scroll(1,2)
local region = mv.graphics.region {x = 0, y = 0, width = 5, height = 5}
local Actor = mv.process(function(self)
  assert(self.scroll == scroll and self.region == region)
  while self.scroll == scroll do mv.frame() end
  assert(self.region ~= region)
  local updated_scroll, updated_region = mv.graphics.scroll(5,6), mv.graphics.region {x = 7, y = 8, width = 9, height = 10}
  self.region = updated_region; self.scroll = updated_scroll
  assert(self.region == updated_region and self.scroll == updated_scroll)
  while self.region ~= nil do mv.frame() end
  assert(self.scroll == nil)
  assert(not pcall(function() self.region = false end))
  assert(not pcall(function() self.scroll = {} end))
  local zero = mv.graphics.scroll(0,0)
  self.scroll = zero; assert(self.scroll == zero)
  self.scroll = nil; assert(self.scroll == nil)
end)
mv.start(function() Actor {region = region, scroll = scroll} end)
