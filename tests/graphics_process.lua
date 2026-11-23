-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local graphics, collision = mv.graphics, mv.collision
assert(mv.circle == graphics.circle and mv.rect == graphics.rect)
local circle = graphics.circle {radius = 2, color = {80, 190, 255}}
local box = graphics.rect {width = 4, height = 2, pivot = "top-left", color = {255, 165, 70, 200}}
assert(graphics.circle {radius = 2.5})
assert(graphics.rect {width = 3.5, height = 1.25, pivot = "center"})
for _, invalid in ipairs({0, -1, 1e-310, math.huge, 0/0, "2", false}) do
  assert(not pcall(graphics.circle, {radius = invalid}))
  assert(not pcall(graphics.rect, {width = invalid, height = 2}))
  assert(not pcall(graphics.rect, {width = 2, height = invalid}))
end
assert(not pcall(graphics.rect, {width = 2}))
assert(not pcall(graphics.circle, {}))
assert(not pcall(graphics.circle, {radius = 2, pivot = "center"}))
assert(not pcall(graphics.rect, {width = 2, height = 2, pivot = "other"}))
assert(not pcall(graphics.rect, {width = 2, height = 2, pivot = "center\0"}))
assert(not pcall(graphics.circle, {radius = 2, color = {1, 2, 256}}))
assert(not pcall(graphics.circle, {radius = 2, color = {1, 2}}))
assert(not pcall(graphics.circle, {radius = 2, extra = 1}))
assert(not pcall(function() circle.radius = 3 end))
local scroll = graphics.scroll(100, -20)
assert(not pcall(graphics.scroll, math.huge, 0))
assert(not pcall(graphics.scroll, 0/0, 0))
assert(not pcall(function() scroll.x = 4 end))
local scroll_actor = mv.process(function() while true do mv.frame() end end,
                                {defaults = {scroll = scroll}})
local region = graphics.region {x = -5, y = 0, width = 10, height = 10}
assert(not pcall(graphics.region, 0, 0, 0, 1))
assert(not pcall(graphics.region, 0, 0, 1.5, 1))
assert(not pcall(graphics.region, math.huge, 0, 1, 1))
assert(not pcall(function() region.x = 4 end))
local region_actor = mv.process(function() while true do mv.frame() end end,
                                {defaults = {region = region}})
local world = mv.world_create()
mv.world_add_rect(world, {x = 10, y = 0, width = 10, height = 10})
mv.world_add_rect(world, {x = 8, y = 0, width = 10, height = 10})
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
  r.region = graphics.region {x = 100, y = 100, width = 1, height = 1}
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
  assert(mv.world_collision(world, a) == 1 and mv.world_collision(world, b) == nil)
  assert(not pcall(mv.world_collision, world, {}))
  assert(mv.world_collision(world, b, {x = 9}) == 1 and b.x == 0)
  assert(mv.world_move(world, b, 100, 0).hit and b.x == 6)
  b.collider = false
  assert(mv.world_collision(world, b) == nil)
  b.collider = collision.circle(1, {offset_x = 3})
  b.x = 4
  assert(mv.world_collision(world, b) == nil)
  b.x = 7
  assert(mv.world_collision(world, b) == 1)
  b.collider = nil
  b.graph = box
  b.x, b.y = 8, 4
  assert(mv.world_collision(world, b) == 1)
  b.x, b.y, b.angle = 8, 0, 90
  assert(mv.world_collision(world, b) == nil)
  b.x = 11
  assert(mv.world_collision(world, b) == 1)
  b.graph = nil
  assert(mv.world_collision(world, b) == nil)
  mv.signal(a, "sleep")
  assert(mv.world_collision(world, a) == 1)
  mv.signal(a, "freeze")
  assert(mv.world_collision(world, a) == 1)
  mv.kill(a)
  assert(not pcall(mv.world_collision, world, a))
  mv.kill(b)
  local weak = setmetatable({}, {__mode = "v"})
  do
    local graphic = graphics.circle {radius = 3}
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
