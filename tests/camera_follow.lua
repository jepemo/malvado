-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
mv.import_globals()
assert(camera_follow == mv.camera.follow)
local Target = process(function() while true do frame() end end)
local function position(camera, x, y)
  local actual_x, actual_y = mv.camera.position(camera)
  assert(actual_x == x and actual_y == y,
    tostring(actual_x) .. "," .. tostring(actual_y) .. " expected " .. x .. "," .. y)
end
local Controller = process(function()
  local first = Target({x = 100, y = 80})
  local second = Target({x = 200, y = 160})
  camera_follow(first)
  frame()
  position(nil, 50, 40)
  first.x = 110
  frame()
  position(nil, 60, 40)
  assert(first.x == 110 and first.y == 80)
  signal(first, "sleep")
  first.x = 120
  frame()
  position(nil, 70, 40)
  signal(first, "freeze")
  first.y = 90
  frame()
  position(nil, 70, 50)
  -- Invalid requests preserve both position and active following.
  assert(not pcall(camera_set, math.huge, 0))
  first.x = 130
  frame()
  position(nil, 80, 50)
  -- Invalid target position retains the last valid position, then recovers.
  first.x = math.huge
  frame()
  position(nil, 80, 50)
  first.x = 140
  frame()
  position(nil, 90, 50)
  camera_set(3, 4)
  first.x = 200
  frame()
  position(nil, 3, 4)
  camera_follow(first)
  camera_follow(second)
  frame()
  position(nil, 150, 120)
  camera_move(1, 2)
  second.x = 300
  frame()
  position(nil, 151, 122)
  local camera = camera_create(0, 0, {viewport = region(10, 20, 40, 30)})
  camera:follow(second)
  frame()
  position(camera, 280, 145)
  local same = camera
  camera:set(5, 6)
  second.y = 200
  frame()
  position(same, 5, 6)
  camera:follow(first)
  frame()
  position(camera, 180, 75)
  camera_follow(first)
  frame()
  position(nil, 150, 50)
  kill(first)
  frame()
  position(nil, 150, 50)
  position(camera, 180, 75)
  assert(not pcall(camera_follow, first))
  assert(not pcall(camera_follow, {}))
  local replacement = Target({x = 0, y = 0})
  frame()
  position(nil, 150, 50) -- No accidental follow of a new process.
  camera:follow(second)
  camera:destroy()
  second.x = 500
  frame()
  assert(not pcall(camera.follow, camera, second))
  -- Weak camera indexing must not keep an unused camera alive.
  local temporary = camera_create(0, 0)
  temporary:follow(second)
  local weak = setmetatable({temporary}, {__mode = "v"})
  temporary = nil
  collectgarbage("collect")
  collectgarbage("collect")
  assert(weak[1] == nil)
  local Request = process(function(self) camera_set(self.x, self.y) end)
  Request({x = 1, y = 2})
  Request({x = 7, y = 8})
  frame()
  frame()
  position(nil, 7, 8) -- Last scheduler request wins within the same tick.
  kill(second)
  kill(replacement)
end)
start(function() Controller() end, {width = 100, height = 80})
