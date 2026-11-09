-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
mv.import_globals()
local atlas = load_asset("examples/aseprite/sheet.json")
local library = load_asset("examples/fpg/graphics.fpg")
local flash = load_asset("tests/fixtures/animated-atlas.json")
local walk = animation(atlas, "bounce")
assert(walk == atlas:animation("bounce") and walk == mv.graphics.animation(atlas, "bounce"))
assert(walk ~= animation(atlas, "bounce", {loop = false}))
assert(not pcall(function() walk.fps = 1 end))
assert(not pcall(animation, "bounce")) -- No scheduled context.
assert(not pcall(animation, atlas, "missing"))
assert(not pcall(animation, atlas, "bounce", {fps = 8}))
assert(not pcall(animation, library, {}))
assert(not pcall(animation, library, {7, 999}))
assert(not pcall(animation, library, {7, 42}, {fps = 0}))
assert(not pcall(animation, library, {7}, {fps = math.huge}))
assert(not pcall(animation, library, {7}, {other = 1}))
assert(not pcall(animation, library, {7}, {loop = 1}))
assert(not pcall(animation, library, {[1] = 7, key = 42}))
assert(not pcall(animation, library, {7.5}))
assert(not pcall(walk.sample, walk, -1))
assert(not pcall(walk.sample, walk, math.huge))
local custom = animation(library, {7, 42, 7}, {fps = 10, loop = false})
assert(custom:sample(0) ~= nil and custom:sample(100) ~= nil)
local elapsed = mv.graphics.animation_elapsed
local Actor = process(function(self) while true do frame() end end)
local Controller = process(function(self)
  assert(not pcall(animation, "bounce")) -- Missing file context.
  self.file = atlas
  assert(animation("bounce") == walk)
  assert(animation({1, 2}, {fps = 8}))
  self.file = library
  assert(animation({7, 42}, {fps = 8}))
  local a = Actor({graph = walk, fps = 5})
  frame()
  frame()
  assert(elapsed(a) > 0)
  local b = Actor({graph = walk})
  local old = elapsed(a)
  frame()
  assert(elapsed(a) > old and elapsed(a) > elapsed(b))
  local current = elapsed(a)
  a.graph = animation(atlas, "bounce")
  assert(elapsed(a) == current) -- Same logical definition does not restart.
  a.graph = animation(atlas, "single")
  assert(elapsed(a) == 0)
  frame()
  frame()
  assert(elapsed(a) > 0)
  signal(a, "freeze")
  current = elapsed(a)
  for i = 1, 10 do frame() end
  assert(elapsed(a) == current)
  signal(a, "sleep")
  for i = 1, 10 do frame() end
  assert(elapsed(a) == current)
  signal(a, "wakeup")
  frame()
  assert(elapsed(a) > current)
  a.graph = custom
  assert(elapsed(a) == 0)
  for i = 1, 30 do frame() end
  assert(elapsed(a) == 300)
  a.graph = graph(library, 7)
  assert(elapsed(a) == nil)
  -- Native Aseprite timing and forward/ping-pong expanded IDs agree with sampling.
  for _, time in ipairs({0, 99, 100, 299, 300, 599, 600, 799, 800}) do
    local id = mv.animation_frame(atlas, "bounce", time)
    local info = mv.sprite_info(atlas, id)
    local probe = Actor({graph = walk:sample(time), x = 0, y = 0})
    local world = world_create()
    world:add_rect({x = -info.pivot_x, y = -info.pivot_y, width = 1, height = 1})
    assert(world:collision(probe))
    kill(probe)
  end
  -- A fixed union keeps collision geometry stable across distinct frame pivots.
  local pivot = load_asset("tests/fixtures/aseprite/animated-pivot.json")
  local animated = Actor({graph = animation(pivot, "default"), x = 200, y = 200})
  local world = world_create()
  world:add_rect({x = 198, y = 201, width = 1, height = 1})
  assert(world:collision(animated) == 1)
  for i = 1, 20 do frame(); assert(world:collision(animated) == 1) end
  animated.collider = false
  assert(world:collision(animated) == nil)
  animated.collider = mv.collision.circle(1)
  assert(world:collision(animated) == nil)
  animated.collider = nil
  assert(world:collision(animated) == 1)
  kill(animated)
  -- Unreferenced descriptor/library cycles must be collectable.
  local transient = load_asset("tests/fixtures/animated-atlas.json")
  local definition = animation(transient, "flash")
  local weak = setmetatable({definition, transient}, {__mode = "v"})
  local c = Actor({graph = definition})
  transient, definition = nil, nil
  collectgarbage("collect")
  assert(weak[1] and weak[2])
  c.graph = nil
  collectgarbage("collect")
  collectgarbage("collect")
  assert(weak[1] == nil and weak[2] == nil)
  kill(c)
  kill(a)
  kill(b)
end)
start(function() Controller() end)
