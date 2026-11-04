-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local world = mv.world.create()
world:add_rect({x = 10, y = 0, width = 10, height = 10})
local shape = mv.collision.circle(2, {offset_x = 1})
local actor = mv.process(function() while true do mv.frame() end end)
local controller = mv.process(function()
  local a = actor({x = 9, y = 5, collider = shape})
  local empty = actor({x = 10, y = 5})
  assert(world:collision(a) == 1)
  assert(world:collision(a, {x = 0}) == nil)
  assert(a.x == 9 and a.y == 5)
  assert(world:collision(a, {x = 9, y = 5}) == 1)
  assert(world:collision(empty) == nil)
  a.collider = false
  assert(world:collision(a) == nil)
  assert(not pcall(world.collision, world, a, {x = math.huge}))
  a.collider = shape
  mv.signal(a, "sleep")
  assert(world:collision(a) == 1)
  mv.signal(a, "freeze")
  assert(world:collision(a) == 1)
  a.size = 0
  assert(not pcall(world.collision, world, a))
  a.size = 1
  assert(not pcall(world.collision, world, a, {z = 1}))
  local moving = actor({x = 0, y = 5, collider = mv.collision.circle(1)})
  local result = world:move(moving, 100, 0)
  assert(result.hit and result.id == 1 and result.moved_x == 9)
  assert(result.moved_y == 0 and result.normal_x == -1 and result.normal_y == 0)
  assert(moving.x == 9 and moving.y == 5)
  assert(world:move(moving, 1, 0).hit and moving.x == 9)
  assert(not world:move(moving, 0, 1).hit and moving.y == 6)
  assert(not world:move(moving, -1, -1).hit and moving.x == 8 and moving.y == 5)
  moving.x = 11
  assert(not pcall(world.move, world, moving, 1, 0) and moving.x == 11)
  assert(not pcall(world.move, world, moving, math.huge, 0))
  local corner = actor({x = 0, y = 10.5, collider = mv.collision.circle(1)})
  local corner_result = world:move(corner, 100, 0)
  assert(corner_result.hit and corner_result.normal_x < 0 and corner_result.normal_y > 0)
  assert(world:collision(corner) == nil)
  mv.kill(corner)
  local rectangle = actor({x = 0, y = 5, angle = 45, collider = mv.collision.rect(2, 2)})
  assert(world:move(rectangle, 100, 0).hit and rectangle.x < 9)
  mv.kill(rectangle)
  empty.collider = false
  assert(not pcall(world.move, world, empty, 1e308, 0) and empty.x == 10)
  local free = world:move(empty, 100, 0)
  assert(not free.hit and free.moved_x == 100 and empty.x == 110)
  mv.kill(moving)
  empty.collider = nil
  empty.graph = {}
  assert(not pcall(world.collision, world, empty))
  empty.collider = shape
  empty.x = 9
  assert(world:collision(empty) == 1)
  mv.kill(a)
  assert(not pcall(world.collision, world, a))
  mv.kill(empty)
end)
mv.start(function() controller() end)
