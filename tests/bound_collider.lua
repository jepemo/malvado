-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local Idle = mv.process(function() while true do mv.frame() end end)
local rect = mv.graphics.rect(4, 2)
local circle = mv.graphics.circle(1)
local Actor = mv.process(function(self)
  local b = Idle {x = 2, graph = circle}
  local c = Idle {x = 7.75, graph = circle}
  local function trace(phase)
    local hits = mv.process.collisions(self, Idle)
    assert(#hits <= 1)
    print(phase .. " hits=" .. (#hits == 0 and "-" or hits[1].id))
  end
  trace("initial")
  self.x = 10; trace("position")
  self.angle = 90; trace("rotation")
  self.size = 2; trace("scale")
  self.collider = mv.collision.circle(.25); trace("override")
  self.collider = false; trace("disabled")
  self.collider = nil; trace("restored")
  self.graph = nil; trace("cleared")
  self.graph = rect; trace("reassigned")
  self.x, self.angle, self.size = 0, 0, 1
  local world = mv.world.create()
  world:add_rect {x = 10, y = -10, width = 1, height = 20}
  assert(world:move(self, 100, 0).hit)
  trace("world")
  mv.signal(b, "sleep"); mv.signal(c, "freeze"); trace("inactive")
  c.size = 0; trace("invalid-target")
  assert(not pcall(mv.process.collision, self, c))
  mv.kill(b); mv.kill(c)
end)
mv.start(function() Actor {graph = rect} end)
