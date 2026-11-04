-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local proc, graphics, collision = mv.process, mv.graphics, mv.collision
local circle = graphics.circle(2)
local Enemy = proc(function() while true do proc.frame() end end,
                   {defaults = {graph = circle}})
local Other = proc(function() while true do proc.frame() end end)
local controller = proc(function()
  local player = Other({graph = circle})
  local first = Enemy({x = 1})
  local second = Enemy({x = 2})
  local far = Enemy({x = 100})
  local list = proc.collisions(player, Enemy)
  assert(#list == 2 and list[1] == first and list[2] == second)
  assert(list ~= proc.collisions(player, Enemy))
  assert(proc.collision(player, Enemy) == first)
  assert(proc.collision(player, first) == first)
  local one = proc.collisions(player, first)
  assert(#one == 1 and one[1] == first)
  player.collider = false
  assert(proc.collision(player, Enemy) == nil and #proc.collisions(player, Enemy) == 0)
  player.collider = nil
  assert(proc.collision(player, player) == nil)
  assert(#proc.collisions(player, player) == 0)
  assert(not pcall(proc.collision, player, function() end))
  first.x = 4 -- Touching is not overlap.
  assert(proc.collision(player, Enemy) == second)
  first.x = 1
  mv.signal(first, "sleep")
  mv.signal(second, "freeze")
  assert(proc.collision(player, Enemy) == first)
  assert(#proc.collisions(player, Enemy) == 2)
  first.collider = false
  assert(proc.collision(player, Enemy) == second)
  first.collider = collision.circle(1, {offset_x = 1})
  first.x = 2
  assert(proc.collision(player, first) == nil)
  first.x = 1
  assert(proc.collision(player, first) == first)
  first.size = 0
  assert(not pcall(proc.collision, first, player))
  assert(not pcall(proc.collision, player, first))
  assert(proc.collision(player, Enemy) == second)
  first.size = 1
  assert(proc.collision(player, first) == first)
  first.graph = nil
  first.collider = nil
  assert(proc.collision(player, first) == nil)
  first.graph = circle
  assert(proc.collision(player, first) == first)
  local world = mv.world.create()
  world:add_rect({x = 10, y = -10, width = 1, height = 20})
  local moved = proc(function() while true do proc.frame() end end)({graph = circle})
  assert(world:move(moved, 100, 0).hit and moved.x == 8)
  assert(proc.collision(moved, player) == nil)
  moved.x = 1
  assert(proc.collision(moved, player) == player)
  mv.kill(first)
  assert(proc.collision(player, first) == nil and #proc.collisions(player, first) == 0)
  assert(proc.collision(player, Enemy) == second)
  mv.kill(second)
  assert(proc.collision(player, Enemy) == nil)
  local many = {}
  for i = 1, 100 do many[i] = Enemy({x = 1}) end
  local hits = proc.collisions(player, Enemy)
  assert(#hits == 100)
  for i = 1, 100 do assert(hits[i] == many[i]); mv.kill(many[i]) end
  assert(proc.collision(player, Enemy) == nil)
  local rotated = Other({graph = graphics.rect(4, 2, {pivot = "top-left"}), x = 10, angle = 90})
  local sensor = Other({x = 8.5, y = .5, collider = collision.rect(1, 1)})
  assert(proc.collision(rotated, sensor) == sensor)
  rotated.angle = 0
  assert(proc.collision(rotated, sensor) == nil)
  rotated.angle = 90
  assert(proc.collision(rotated, sensor) == sensor)
  mv.kill(rotated)
  mv.kill(sensor)
  mv.kill(moved)
  mv.kill(far)
  mv.kill(player)
  assert(not pcall(proc.collision, player, Enemy))
end)
mv.start(function() controller() end)
