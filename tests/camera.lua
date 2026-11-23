-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
mv.import_globals()
assert(camera_create == mv.camera_create and camera_set == mv.camera_set)
assert(camera_move == mv.camera_move)
assert(region == mv.region and region == mv.graphics.region)
local x, y = mv.camera_position()
assert(x == 0 and y == 0)
camera_set(100, 50)
camera_move(-10, 5)
x, y = mv.camera_position()
assert(x == 90 and y == 55)
local clip = region {x = -40, y = -60, width = 300, height = 240}
local camera = camera_create {x = 10.5, y = 20, viewport = clip}
x, y = camera_position(camera)
assert(x == 10.5 and y == 20)
local coordinate_limit = 0x1.fffffep121 -- FLT_MAX / 64.
local x_only = camera_create {x = coordinate_limit}
x, y = camera_position(x_only)
assert(x == coordinate_limit and y == 0)
local y_only = camera_create {y = -20}
x, y = camera_position(y_only)
assert(x == 0 and y == -20)
camera_destroy(x_only)
camera_destroy(y_only)
camera_set(camera, 30, 40)
camera_move(camera, 2, -3)
x, y = camera_position(camera)
assert(x == 32 and y == 37)
assert(not pcall(camera_create))
assert(not pcall(camera_create, false))
assert(not pcall(camera_create, 0, 0))
assert(not pcall(camera_create, {}, nil))
assert(not pcall(camera_create, {x = 0/0}))
assert(not pcall(camera_create, {y = math.huge}))
assert(not pcall(camera_create, {x = 0x1p122})) -- In float range but beyond FLT_MAX / 64.
assert(not pcall(camera_create, {y = "0"}))
assert(not pcall(camera_create, {unknown = true}))
assert(not pcall(camera_create, {[1] = 0}))
assert(not pcall(camera_create, {viewport = {}}))
assert(not pcall(camera_create, {viewport = scroll(0, 0)}))
assert(not pcall(camera_position, {}))
assert(not pcall(camera_destroy))
assert(not pcall(camera_destroy, {}))
assert(not pcall(region, 0, 0, 1, 1))
local rectangle = {x = 0, y = 0, width = 1, height = 1}
for _, field in ipairs({"x", "y", "width", "height"}) do
  local value = rectangle[field]
  rectangle[field] = nil
  assert(not pcall(region, rectangle))
  rectangle[field] = value
end
rectangle.extra = 1
assert(not pcall(region, rectangle))
rectangle.extra = nil
local int_min, int_max = -0x80000000, 0x7fffffff
assert(region {x = int_min, y = int_max, width = int_max, height = int_max})
for field, values in pairs({
  x = {int_min - 1, int_max + 1, "0"},
  y = {0.5, math.huge},
  width = {0, -1, int_max + 1, 1.5},
  height = {0, -1, int_max + 1},
}) do
  local original = rectangle[field]
  for _, value in ipairs(values) do
    rectangle[field] = value
    assert(not pcall(region, rectangle))
  end
  rectangle[field] = original
end
assert(not pcall(function() clip.width = 1 end))
assert(not pcall(camera_set, math.huge, 0))
assert(not pcall(camera_move, 1e308, 0))
x, y = mv.camera_position()
assert(x == 90 and y == 55)
local closed = camera_create {}
x, y = camera_position(closed)
assert(x == 0 and y == 0)
camera_destroy(closed)
camera_destroy(closed)
assert(not pcall(camera_set, closed, 0, 0))
assert(not pcall(camera_position, closed))
local Actor = process(function(self)
  assert(self.camera == camera)
  assert(self.x == 100 and self.y == 200 and self.z == 1)
  self.camera = false
  assert(self.camera == false)
  self.camera = nil
  assert(self.camera == nil)
  assert(not pcall(function() self.camera = true end))
  assert(not pcall(function() self.camera = closed end))
  assert(self.camera == nil)
  self.camera = camera
  self.camera = nil
  assert(self.region == clip)
  assert(not pcall(function() self.region = region {x = 0, y = 0, width = 0, height = 1} end))
  assert(self.region == clip)
  self.region = nil
  assert(self.region == nil)
end, {defaults = {camera = camera, region = clip}})
start(function()
  assert(not pcall(Actor, {camera = {}}))
  assert(not pcall(Actor, {camera = closed}))
  Actor({x = 100, y = 200, z = 1})
  Actor({x = 100, y = 200, z = 1, region = clip})
end)
