-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
assert(camera_create == nil)
assert(not pcall(mv.load_image, "fixtures/red.png"))

local viewport = mv.region {x = 0, y = 0, width = 10, height = 10}
local camera = mv.camera_create {viewport = viewport}
local other_camera = mv.camera_create {viewport = viewport}
local world = mv.world_create()
local other_world = mv.world_create()
local shape = mv.collision.circle {radius = 1}
local rect = {x = 10, y = 0, width = 10, height = 10}
local transform = {x = 11, y = 5}
local sweep = {y = 5, dx = 100, dy = 0}

local function same_results(a, b)
  for key, value in pairs(a) do assert(b[key] == value, key) end
  for key, value in pairs(b) do assert(a[key] == value, key) end
end

assert(getmetatable(camera) == "malvado camera")
assert(camera.set == other_camera.set and world.move == other_world.move)
assert(select("#", camera:set(10, 20)) == 0)
assert(select("#", mv.camera_set(other_camera, 10, 20)) == 0)
assert(select("#", camera.move(camera, 1, -2)) == 0)
assert(select("#", mv.camera_move(other_camera, 1, -2)) == 0)
same_results({camera:position()}, {mv.camera_position(other_camera)})
same_results({camera.position(camera)}, {11, 18})
assert(not pcall(camera.position, nil) and not pcall(camera.move, 1, 2))
assert(not pcall(camera.set, 1, 2) and not pcall(camera.set, world, 1, 2))
assert(not pcall(camera.set, camera, math.huge, 0))
assert(camera:position() == 11 and mv.camera_position() == 0)

assert(world:add_rect(rect) == mv.world_add_rect(other_world, rect))
assert(world.add_rect(world, rect) == mv.world_add_rect(other_world, rect))
assert(world:overlap(shape, transform) == mv.world_overlap(other_world, shape, transform))
assert(world.overlap(world, shape) == nil)
same_results(world:sweep(shape, sweep), mv.world_sweep(other_world, shape, sweep))
same_results(world.sweep(world, shape, sweep), mv.world_sweep(world, shape, sweep))
assert(not pcall(world.overlap, camera, shape))
assert(not pcall(world.sweep, world, shape, {x = 0}))
assert(not pcall(world.add_rect, world, {width = 0}))

local actor = mv.process(function()
  while true do mv.frame() end
end)
local controller = mv.process(function()
  local first = actor {x = 0, y = 5, collider = shape}
  local second = actor {x = 0, y = 5, collider = shape}
  assert(select("#", camera:follow(first)) == 0)
  assert(select("#", mv.camera_follow(other_camera, first)) == 0)
  camera.follow(camera, first)
  assert(not pcall(camera.follow, first))
  assert(not pcall(world.collision, nil, first))
  assert(not pcall(world.collision, world, nil))
  assert(not pcall(world.move, 1, 2))
  assert(world:collision(first, transform) == mv.world_collision(other_world, second, transform))
  assert(world.collision(world, first) == nil)
  same_results(world:move(first, 100, 0), mv.world_move(other_world, second, 100, 0))
  same_results(world.move(world, first, 0, 1), mv.world_move(other_world, second, 0, 1))
  assert(first.x == second.x and first.y == second.y)
  mv.frame()
  same_results({camera:position()}, {mv.camera_position(other_camera)})
  same_results({camera:position()}, {first.x - 5, first.y - 5})

  mv.kill(first)
  assert(not pcall(camera.follow, camera, first))
  assert(not pcall(world.collision, world, first))
  assert(not pcall(world.move, world, first, 1, 0))
  mv.kill(second)
  assert(select("#", camera:destroy()) == 0)
  assert(select("#", camera.destroy(camera)) == 0)
  assert(select("#", mv.camera_destroy(other_camera)) == 0)
  assert(not pcall(camera.position, camera))
  assert(not pcall(mv.camera_position, other_camera))
  collectgarbage("collect")
end)

if mv.animation then
  local library = mv.load_asset("../examples/aseprite/sheet.json")
  local clip = mv.animation(library, "bounce")
  assert(clip:sample(0) == mv.animation_sample(clip, 0))
  assert(clip.sample(clip, 500) == mv.animation_sample(clip, 500))
  assert(not pcall(clip.sample, world, 0))
  assert(not pcall(clip.sample, clip, -1))
end

mv.import_globals()
assert(camera_set == mv.camera_set and world_create == mv.world_create)
local imported = camera_create {}
assert(imported:position() == camera_position(imported))
imported:destroy()
mv.start(function() controller() end)
