-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local function trace(phase, camera)
  local x, y = mv.camera_position(camera)
  print(string.format("%s x=%.0f y=%.0f", phase, x, y))
end
local Actor = mv.process(function() while true do mv.frame() end end)
local Controller = mv.process(function()
  local target = Actor {x = 100, y = 80}
  mv.camera_follow(target)
  mv.frame()
  trace("follow")
  mv.signal(target, "freeze")
  target.x, target.y = 120, 90
  mv.frame()
  trace("frozen")
  local camera = mv.camera_create {x = 0, y = 0, viewport = mv.graphics.region {x = 10, y = 20, width = 40, height = 60}}
  mv.camera_follow(camera, target)
  mv.frame()
  trace("viewport", camera)
  mv.kill(target)
  mv.frame()
  trace("ended", camera)
end)
mv.start(function() Controller() end, {width = 100, height = 80})
