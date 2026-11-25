-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local function rejects(fn, ...)
  local ok, message = pcall(fn, ...)
  assert(not ok, "invalid call succeeded")
  return message
end

rejects(mv.import_globals, nil)
assert(rand == nil)
mv.import_globals()
assert(rand(7.0, 7) == 7)
rejects(rand, "7", 7)
rejects(rand, 7, 7, nil)
rejects(rand_seed, 1.0)
assert(not mv.input.key(KEY_1))
rejects(key, 1)
rejects(mv.input.key, KEY_1, nil)
rejects(pad_axis, "0", PAD_LEFT_X)
rejects(mouse_position, nil)
rejects(text_input_next, nil)
rejects(time_seconds, nil)
rejects(load_asset, "missing.png", nil, nil)
rejects(load_music, "missing.ogg", {loop = 1})
rejects(font, {size = "20"})
rejects(mv.region, {x = 0, y = 0, width = 10, height = 10}, nil)
rejects(mv.scroll, "1", 2)
rejects(mv.scroll, 1, math.huge)

local directory = get_assets_dir()
rejects(set_assets_dir, ".", nil)
assert(get_assets_dir() == directory)
set_assets_dir(nil)
rejects(start, function() error("invalid start initialized the game") end, nil, nil)

local world = world_create()
local shape = collision.rect {width = 2, height = 2}
local wall = {x = 0, y = 0, width = 9, height = 9}
rejects(world.add_rect, world, wall, nil)
rejects(world.add_rect, world, {x = "0", y = 0, width = 9, height = 9})
assert(world:overlap(shape, {x = 2, y = 2}) == nil)
assert(world:add_rect(wall) == 1)
assert(world:overlap(shape, {x = 2, y = 2}) == 1)
rejects(world.overlap, world, shape, {x = "2", y = 2})
rejects(world.overlap, world, shape, {x = math.huge})
rejects(collision.overlap, shape, shape, nil)
assert(collision.overlap(shape, shape))

local camera = camera_create {}
camera:set(3.5, 4)
rejects(camera.move, camera, "1", 1)
rejects(camera_set, camera, 0, 0, nil)
rejects(camera.destroy, camera, nil)
assert(camera:position() == 3.5)
rejects(camera.position)

local Actor = process(function(self)
  assert(self.fps == 0)
  local message = {payload = math.huge}
  self:send(message)
  rejects(recv, self, nil)
  rejects(self.recv)
  assert(self:recv() == message and recv() == nil)
  self.custom = math.huge
  assert(self.custom == message.payload)

  self.world = world
  self.x, self.y = 20, 30
  rejects(world.move, world, self, "1", 2)
  rejects(world_collision, world, self, {x = "5"})
  rejects(world.collision, world)
  assert(self.x == 20 and self.y == 30)
  local nested = coroutine.create(function()
    rejects(world_move, 1, 2)
    rejects(recv)
    assert(world:move(self, 1, 2).moved_x == 1)
  end)
  assert(coroutine.resume(nested))
  assert(self.x == 21 and self.y == 32)

  set_priority(4.0)
  rejects(self.set_priority, self, "5")
  assert(self.priority == 4)
  rejects(frame, nil)
  assert(snapshot().tick == 1)

  local f = font {size = 20.0}
  rejects(draw_text, {text = 10, x = 0, y = 0})
  rejects(draw_text, {text = "hello", x = "0", y = 0})
  rejects(text_size, f, 10)
  local id = write {text = "hello", x = 0, y = 0, font = f}
  rejects(change_text, id, {text = "new", x = "1"})
  rejects(text_size, id, nil)
  rejects(delete_text, id, nil)
  delete_text(id)
end)

start(function()
  local width, height = window_size()
  rejects(set_window_size, 320, 240, nil)
  rejects(set_window_size, 320.0, 240)
  rejects(set_fps, 30, nil)
  rejects(set_fps, 30.0)
  local new_width, new_height = window_size()
  assert(width == new_width and height == new_height)
  rejects(process.define, function() end, nil, nil)
  rejects(Actor, nil, nil, nil)
  rejects(spawn, Actor, nil, nil, nil)
  assert(#find(Actor) == 0)
  spawn(Actor)
end)
