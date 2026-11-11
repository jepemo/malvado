-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local owned = mv.camera.create(5, 6)
local Actor = mv.process(function(self)
  assert(self.camera == owned)
  while mv.camera.position(owned) == 5 do mv.frame() end
  local x, y = owned:position()
  assert(x == 20 and y == 30)
  owned:move(1, 2)
  while self.camera == owned do mv.frame() end
  x, y = self.camera:position()
  assert(x == 40 and y == 50)
  self.camera:set(41, 52)
  local weak = setmetatable({self.camera}, {__mode = "v"})
  self.camera = false
  collectgarbage("collect"); collectgarbage("collect")
  assert(weak[1] == nil)
  while self.camera == false do mv.frame() end
  local borrowed = self.camera
  x, y = borrowed:position()
  assert(x == 41 and y == 52)
  self.x = 2
  while self.y ~= 1 do mv.frame() end
  local Probe = mv.process(function() while true do mv.frame() end end)
  local count = #mv.snapshot().processes
  assert(not pcall(Probe, {camera = borrowed}))
  assert(#mv.snapshot().processes == count)
  assert(not pcall(borrowed.position, borrowed))
  self.camera = nil
  assert(self.camera == nil)
  assert(mv.camera.position() == 0)
  owned:destroy()
end)
mv.start(function() Actor {camera = owned} end)
