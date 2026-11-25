-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
mv.import_globals()
local atlas = load_asset("../examples/aseprite/sheet.json")
local library = load_asset("../examples/fpg/graphics.fpg")
local walk = animation(atlas, "bounce")
assert(walk == mv.animation(atlas, "bounce") and walk == mv.graphics.animation(atlas, "bounce"))
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
assert(not pcall(animation_sample, walk, -1))
assert(not pcall(animation_sample, walk, math.huge))
local custom = animation(library, {7, 42, 7}, {fps = 10, loop = false})
assert(animation_sample(custom, 0) ~= nil and animation_sample(custom, 100) ~= nil)
local named_once = animation(atlas, "bounce", {loop = false})
for _, loop in ipairs({true, false}) do
  local definition = loop and walk or named_once
  for _, time in ipairs({0, 99.9999, 100, 100.0001, 299.9999, 300, 300.0001,
                        599.9999, 600, 600.0001, 799.9999, 800, 800.0001, 1e12}) do
    local id = atlas_animation_frame(atlas, "bounce", time, {loop = loop})
    assert(definition:sample(time) == graph(atlas, id))
    assert(atlas:animation_frame("bounce", time, {loop = loop}) == id)
  end
end
assert(atlas_animation_frame(atlas, "bounce", 800.5) == 1)
assert(atlas_animation_frame(atlas, "bounce", 800.5, {}) == 1)
assert(atlas_animation_frame(atlas, "bounce", 800.5, {loop = false}) == 2)
assert(atlas_animation_color(atlas, "bounce")[4] == 128)
assert(atlas_animation_repeats(atlas, "bounce") == nil)
assert(atlas_animation_color(atlas, "missing") == nil)
assert(atlas_animation_repeats(atlas, "missing") == nil)
for _, time in ipairs({-0.1, math.huge, -math.huge, 0/0, 1e12 + 1, {}}) do
  assert(not pcall(atlas_animation_frame, atlas, "bounce", time))
end
for _, options in ipairs({false, true, 1, "loop", {loop = 1}, {fps = 8}, {[1] = false}}) do
  assert(not pcall(atlas_animation_frame, atlas, "bounce", 100, options))
end
assert(not pcall(atlas_animation_frame, atlas, "missing", 0))
assert(not pcall(atlas_animation_frame, atlas, "bounce\0suffix", 0))

local elapsed = mv.graphics.animation_elapsed
assert(elapsed == mv.animation_elapsed and elapsed == animation_elapsed)
local finished = mv.graphics.animation_finished
assert(finished == mv.animation_finished and finished == animation_finished)
local function fails(fragment, fn, ...)
  local ok, message = pcall(fn, ...)
  assert(not ok and tostring(message):find(fragment, 1, true), tostring(message))
end
fails("running process", elapsed)
fails("running process", finished)
local Actor = process(function(self)
  while true do
    assert(animation_elapsed() == elapsed(self))
    assert(animation_finished() == finished(self) and finished(self) == self:animation_finished())
    frame()
  end
end)
local completed_at
local OneShot = process(function(self)
  self.file = atlas
  self.graph = animation("single", {loop = false})
  while not animation_finished() do
    assert(animation_elapsed() < 200)
    frame()
  end
  assert(animation_elapsed() == 200)
  completed_at = animation_elapsed()
end)
local Controller = process(function(self)
  assert(animation_elapsed() == nil and not animation_finished())
  fails("malvado.process", finished, nil)
  fails("malvado.process", finished, self.id)
  fails("expected animation_finished", finished, self, true)
  fails("expected animation_finished", self.animation_finished, self, true)
  fails("malvado.process", self.animation_finished)
  self.graph = walk
  assert(animation_elapsed() == 0 and not animation_finished())
  -- The assignment tick presents the first frame before playback advances.
  frame()
  frame()
  local time = animation_elapsed()
  assert(time > 0 and time == elapsed(self) and time == elapsed())
  fails("malvado.process", elapsed, nil)
  fails("malvado.process", elapsed, self.id)
  fails("expected animation_elapsed", elapsed, self, true)
  local ok, message = coroutine.resume(coroutine.create(function()
    fails("running process", elapsed)
    assert(elapsed(self) == time)
    fails("running process", finished)
    assert(not finished(self))
  end))
  assert(ok, message)
  assert(elapsed() == time)
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
  assert(not finished(a) and not finished(b))
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
  a.graph = named_once
  assert(elapsed(a) == 0 and not finished(a))
  while elapsed(a) < 600 do frame() end
  assert(elapsed(a) == 600 and not finished(a)) -- Last frame has just started.
  signal(a, "freeze")
  for i = 1, 15 do frame() end
  assert(elapsed(a) == 600 and not finished(a))
  signal(a, "sleep")
  for i = 1, 15 do frame() end
  assert(elapsed(a) == 600 and not finished(a))
  signal(a, "wakeup")
  while elapsed(a) < 800 do
    assert(not finished(a) and not finished(b))
    frame()
  end
  assert(elapsed(a) == 800 and finished(a))
  a.graph = animation(atlas, "bounce", {loop = false})
  assert(elapsed(a) == 800 and finished(a)) -- Cached reassignment preserves completion.
  for _, paused in ipairs({"sleep", "freeze"}) do
    signal(a, paused)
    frame()
    assert(finished(a))
  end
  signal(a, "wakeup")
  frame()
  assert(finished(a))
  b.graph = named_once
  assert(finished(a) and not finished(b)) -- Same definition, independent clocks.
  a.graph = nil
  assert(elapsed(a) == nil and not finished(a))
  a.graph = named_once
  assert(elapsed(a) == 0 and not finished(a))
  a.graph = custom
  assert(elapsed(a) == 0 and not finished(a))
  for i = 1, 30 do frame() end
  assert(elapsed(a) == 300 and finished(a))
  a.graph = graph(library, 7)
  assert(elapsed(a) == nil and not finished(a))
  b.graph = nil
  assert(elapsed(b) == nil and not finished(b))
  b.graph = named_once
  assert(elapsed(b) == 0 and not finished(b))

  local shot = OneShot()
  assert(not finished(shot))
  while alive(shot) do frame() end
  assert(completed_at == 200)
  fails("no longer alive", finished, shot)
  -- Native Aseprite timing and forward/ping-pong expanded IDs agree with sampling.
  for _, time in ipairs({0, 99, 100, 299, 300, 599, 600, 799, 800}) do
    local id = mv.atlas_animation_frame(atlas, "bounce", time)
    local info = mv.sprite_info(atlas, id)
    local probe = Actor({graph = animation_sample(walk, time), x = 0, y = 0})
    local world = world_create()
    world_add_rect(world, {x = -info.pivot_x, y = -info.pivot_y, width = 1, height = 1})
    assert(world_collision(world, probe))
    kill(probe)
  end
  -- A fixed union keeps collision geometry stable across distinct frame pivots.
  local pivot = load_asset("fixtures/aseprite/animated-pivot.json")
  local animated = Actor({graph = animation(pivot, "default"), x = 200, y = 200})
  local world = world_create()
  world_add_rect(world, {x = 198, y = 201, width = 1, height = 1})
  assert(world_collision(world, animated) == 1)
  for i = 1, 20 do frame(); assert(world_collision(world, animated) == 1) end
  animated.collider = false
  assert(world_collision(world, animated) == nil)
  animated.collider = mv.collision.circle {radius = 1}
  assert(world_collision(world, animated) == nil)
  animated.collider = nil
  assert(world_collision(world, animated) == 1)
  kill(animated)
  -- Unreferenced descriptor/library cycles must be collectable.
  local transient = load_asset("fixtures/animated-atlas.json")
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
  fails("no longer alive", elapsed, a)
  fails("no longer alive", finished, a)
  ok, message = coroutine.resume(coroutine.create(function() assert(kill(self)) end))
  assert(ok, message)
  fails("no longer alive", elapsed)
  fails("no longer alive", finished)
end)
start(function()
  local controller = Controller()
  fails("running process", elapsed)
  assert(elapsed(controller) == nil and not finished(controller))
end)
fails("running process", elapsed)
