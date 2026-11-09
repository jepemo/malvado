-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
mv.import_globals()
local function fails(fragment, fn, ...)
  local ok, message = pcall(fn, ...)
  assert(not ok and tostring(message):find(fragment, 1, true), tostring(message))
end
fails("running process", collision, {})
fails("running process", world_move, 1, 0)
fails("expected world_move", world_move, 1)
fails("expected collision", collision)
assert(world_create == mv.world.create and world_move == mv.world.move)
local weak = setmetatable({}, {__mode = "v"})
local function make_world(x)
  local world = world_create()
  world:add_rect({x = x, y = 0, width = 10, height = 20})
  if x == 20 then weak[2] = world end
  return world
end
local Enemy = process(function() while true do frame() end end,
  {defaults = {graph = mv.graphics.circle(1)}})
local peers = {}
local Player = process(function(self)
  local first, second = peers[1], peers[2]
  assert(collision(Enemy) == first)
  assert(collision(first) == process.collision(self, first))
  assert(collision(self) == nil)
  signal(first, "sleep")
  assert(collision(Enemy) == first)
  signal(first, "freeze")
  assert(collision(Enemy) == first)
  kill(first)
  assert(collision(first) == nil and collision(Enemy) == second)
  self.collider = false
  assert(collision(Enemy) == nil)
  self.collider = nil
  assert(collision(Enemy) == second)
  self.collider = mv.collision.rect(2, 2)
  assert(collision(Enemy) == second)
  self.collider = nil
  fails("constructor or handle", collision, {})
  local nested = coroutine.create(function()
    fails("running process", collision, Enemy)
    fails("running process", world_move, 1, 0)
    -- Explicit movement never borrows a coroutine's contextual world.
    assert(not world_move(self.world, self, 0, 0).hit)
  end)
  local ok, message = coroutine.resume(nested)
  assert(ok, message)
  self.x = 0
  local world = self.world
  self.world = nil
  fails("no world", world_move, 1, 0)
  self.world = {}
  fails("world must be", world_move, 1, 0)
  assert(self.x == 0)
  self.world = world
  local hit = world_move(100, 0)
  assert(hit.hit and hit.id == 1 and hit.moved_x == 9 and hit.moved_y == 0)
  assert(hit.normal_x == -1 and hit.normal_y == 0 and self.x == 9)
  self.x = 0
  local explicit = world_move(world, self, 100, 0)
  self.x = 0
  local method = world:move(self, 100, 0)
  for key, value in pairs(hit) do assert(explicit[key] == value and method[key] == value) end
  self.x = 0
  self.world = make_world(20)
  assert(world_move(100, 0).moved_x == 19)
  self.x = 0
  assert(world_move(world, self, 100, 0).moved_x == 9)
  self.x = 0
  self.collider = false
  assert(not world_move(100, 0).hit and self.x == 100)
  self.x = 0
  self.collider = mv.collision.circle(2)
  assert(world_move(100, 0).moved_x == 18)
  self.collider = nil
  self.x = 12 -- Direct coordinates do not invoke world movement.
  assert(self.x == 12)
  self.x = 21
  fails("initial collision", world_move, 1, 0)
  assert(self.x == 21)
  self.x = 0
  fails("invalid movement", world_move, math.huge, 0)
  assert(self.x == 0)
  fails("collision_world", world_move, {}, self, 1, 0)
  fails("malvado.process", world_move, world, {}, 1, 0)
  kill(second)
  fails("no longer alive", world_move, world, second, 1, 0)
  world = nil
  collectgarbage("collect")
  assert(weak[1] == nil)
  assert(weak[2] == self.world)
  self.world = nil
  collectgarbage("collect")
  assert(weak[2] == nil)
end, {defaults = {graph = mv.graphics.circle(1)}})
local game = {}
function game.init()
  peers[1], peers[2] = Enemy({x = 0, y = 5}), Enemy({x = 0, y = 5})
  local world = make_world(10)
  weak[1] = world
  local player = Player({x = 0, y = 5, world = world})
  -- Explicit movement is valid during initialization, outside a process.
  assert(not world_move(world, player, 0, 0).hit)
  fails("running process", collision, Enemy)
end
return game
