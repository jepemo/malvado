-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local graphics, collision = mv.graphics, mv.collision
local circle = graphics.circle(2, {color = {80, 190, 255}})
local box = graphics.rect(4, 2, {pivot = "top-left", color = {255, 165, 70, 200}})
assert(not pcall(graphics.circle, 0))
assert(not pcall(graphics.circle, 1e-310))
assert(not pcall(graphics.rect, 1, math.huge))
assert(not pcall(graphics.circle, 2, {pivot = "top-left"}))
assert(not pcall(graphics.rect, 2, 2, {pivot = "other"}))
assert(not pcall(graphics.circle, 2, {color = {1, 2, 256}}))
assert(not pcall(graphics.circle, 2, {color = {1, 2}}))
assert(not pcall(graphics.circle, 2, {extra = 1}))
assert(not pcall(function() circle.radius = 3 end))
local scroll = graphics.scroll(100, -20)
assert(not pcall(graphics.scroll, math.huge, 0))
assert(not pcall(graphics.scroll, 0/0, 0))
assert(not pcall(function() scroll.x = 4 end))
local scroll_actor = mv.process(function() while true do mv.frame() end end,
                                {defaults = {scroll = scroll}})
local region = graphics.region(-5, 0, 10, 10)
assert(not pcall(graphics.region, 0, 0, 0, 1))
assert(not pcall(graphics.region, 0, 0, 1.5, 1))
assert(not pcall(graphics.region, math.huge, 0, 1, 1))
assert(not pcall(function() region.x = 4 end))
local region_actor = mv.process(function() while true do mv.frame() end end,
                                {defaults = {region = region}})
local world = mv.world.create()
world:add_rect({x = 10, y = 0, width = 10, height = 10})
world:add_rect({x = 8, y = 0, width = 10, height = 10})
local actor = mv.process(function() while true do mv.frame() end end,
                         {defaults = {graph = circle, nested = {count = 0}}})
local controller = mv.process(function()
  assert(not pcall(scroll_actor, {scroll = {}}))
  local scrolled = scroll_actor({graph = circle})
  assert(scrolled.scroll == scroll)
  assert(not pcall(function() scrolled.scroll = false end) and scrolled.scroll == scroll)
  local other = actor({x = 0, y = 0})
  assert(mv.process.collision(scrolled, other) == other)
  scrolled.scroll = nil
  assert(scrolled.scroll == nil)
  mv.kill(scrolled)
  mv.kill(other)
  assert(not pcall(region_actor, {region = {}}))
  local r = region_actor()
  assert(r.region == region)
  r.graph = circle
  local target = actor({x = 0, y = 0})
  r.region = graphics.region(100, 100, 1, 1)
  assert(mv.process.collision(r, target) == target)
  mv.kill(target)
  r.region = region
  assert(not pcall(function() r.region = false end) and r.region == region)
  r.region = nil
  assert(r.region == nil)
  r.region = region
  mv.kill(r)
  local a = actor({x = 9, y = 5})
  local b = actor({x = 0, y = 5})
  assert(a.graph == circle and b.graph == circle and a.nested ~= b.nested)
  assert(world:collision(a) == 1 and world:collision(b) == nil)
  assert(not pcall(world.collision, world, {}))
  assert(world:collision(b, {x = 9}) == 1 and b.x == 0)
  assert(world:move(b, 100, 0).hit and b.x == 6)
  b.collider = false
  assert(world:collision(b) == nil)
  b.collider = collision.circle(1, {offset_x = 3})
  b.x = 4
  assert(world:collision(b) == nil)
  b.x = 7
  assert(world:collision(b) == 1)
  b.collider = nil
  b.graph = box
  b.x, b.y = 8, 4
  assert(world:collision(b) == 1)
  b.x, b.y, b.angle = 8, 0, 90
  assert(world:collision(b) == nil)
  b.x = 11
  assert(world:collision(b) == 1)
  b.graph = nil
  assert(world:collision(b) == nil)
  mv.signal(a, "sleep")
  assert(world:collision(a) == 1)
  mv.signal(a, "freeze")
  assert(world:collision(a) == 1)
  mv.kill(a)
  assert(not pcall(world.collision, world, a))
  mv.kill(b)
  local weak = setmetatable({}, {__mode = "v"})
  do
    local graphic = graphics.circle(3)
    weak[1] = graphic
    local once = mv.process(function(self) assert(self.graph) end, {defaults = {graph = graphic}})
    once()
  end
  mv.frame()
  mv.frame()
  collectgarbage("collect")
  collectgarbage("collect")
  assert(weak[1] == nil)
end)
mv.start(function() controller() end)
