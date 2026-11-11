-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local actor = mv.process(function(self)
  assert(self.x == 4 and math.type(self.x) == "integer" and self.y == 6)
  while self.x ~= 12 do mv.frame() end
  assert(self.y == 8 and self.angle == 90 and self.size == 2)
  self.angle = 180
  self.size = .5
  local world = mv.world.create()
  world:add_rect {x = 20, y = -10, width = 1, height = 40}
  assert(world:move(self, 100, 0).hit and self.x == 19.5 and self.y == 8)
  while self.x ~= -2 do mv.frame() end
  assert(self.y == 5 and self.angle == 270 and self.size == 3)
  self.angle = nil
  self.size = nil
  assert(self.angle == 0 and self.size == 1)
  local sensor = mv.process(function() while true do mv.frame() end end) {
    x = -2, y = 5, collider = mv.collision.rect(2, 2)
  }
  assert(mv.process.collision(self, sensor) == sensor)
  for _, value in ipairs({math.maxinteger, math.mininteger, .25, -0.0}) do
    self.x = value
    assert(self.x == value and math.type(self.x) == math.type(value))
  end
  self.x = nil
  assert(self.x == 0 and math.type(self.x) == "integer")
  for _, value in ipairs({"bad", false, math.huge}) do
    self.x = value
    assert(self.x == value)
    assert(not pcall(world.move, world, self, 1, 0))
  end
  self.x = 0 / 0
  assert(self.x ~= self.x)
  self.x = 2
  self.y = nil
  assert(self.x == 2 and self.y == 0)
  for _, field in ipairs({"angle", "size"}) do
    for _, value in ipairs({math.maxinteger, .25, 0, -1, "bad", false, math.huge}) do
      self[field] = value
      assert(self[field] == value)
    end
    self[field] = nil
  end
  assert(self.angle == 0 and self.size == 1)
  mv.kill(sensor)
end)
mv.start(function() actor {x = 4, y = 6, collider = mv.collision.rect(2, 2)} end)
