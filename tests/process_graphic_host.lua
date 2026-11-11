-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local initial = mv.graphics.rect(2, 2)
local replacement = mv.graphics.rect(4, 2, {pivot = "top-left", color = {0, 255, 0}})
local Actor = mv.process(function(self)
  assert(self.graph == initial)
  local sensor = mv.process(function() while true do mv.frame() end end) {
    x = 4, collider = mv.collision.rect(1, 1)
  }
  assert(mv.process.collision(self, sensor) == nil)
  while self.graph == initial do mv.frame() end
  assert(mv.process.collision(self, sensor) == sensor)
  self.graph = replacement
  assert(self.graph == replacement)
  while self.graph ~= nil do mv.frame() end
  assert(mv.process.collision(self, sensor) == nil)
  self.graph = false
  assert(self.graph == false)
  self.graph = initial
  assert(self.graph == initial)
  self.graph = nil
  assert(self.graph == nil)
  mv.kill(sensor)
end)
mv.start(function() Actor {graph = initial} end)
