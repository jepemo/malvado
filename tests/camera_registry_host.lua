-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
assert(not pcall(mv.camera_create, {x = math.huge}))
assert(not pcall(mv.camera_create, {viewport = {}}))
local temporary = mv.camera_create {}
temporary = nil
collectgarbage("collect")
local owned = mv.camera_create {x = 5, y = 6}
local Actor = mv.process(function(self)
  assert(self.camera == owned)
  while mv.camera_position(owned) == 5 do mv.frame() end
  local x, y = mv.camera_position(owned)
  assert(x == 20 and y == 30)
  mv.camera_move(owned, 1, 2)
  while self.camera == owned do mv.frame() end
  x, y = mv.camera_position(self.camera)
  assert(x == 40 and y == 50)
  mv.camera_set(self.camera, 41, 52)
  local weak = setmetatable({self.camera}, {__mode = "v"})
  self.camera = false
  collectgarbage("collect"); collectgarbage("collect")
  assert(weak[1] == nil)
  while self.camera == false do mv.frame() end
  local borrowed = self.camera
  x, y = mv.camera_position(borrowed)
  assert(x == 41 and y == 52)
  self.x = 2
  while self.y ~= 1 do mv.frame() end
  local Probe = mv.process(function() while true do mv.frame() end end)
  local count = #mv.snapshot().processes
  assert(not pcall(Probe, {camera = borrowed}))
  assert(#mv.snapshot().processes == count)
  assert(not pcall(mv.camera_position, borrowed))
  self.camera = nil
  assert(self.camera == nil)
  assert(mv.camera_position() == 0)
  mv.camera_destroy(owned)
end)
mv.start(function() Actor {camera = owned} end)
