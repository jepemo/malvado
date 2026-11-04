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
  empty.graph = {}
  assert(not pcall(world.collision, world, empty))
  mv.kill(a)
  assert(not pcall(world.collision, world, a))
  mv.kill(empty)
end)
mv.start(function() controller() end)
