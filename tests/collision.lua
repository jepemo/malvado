-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
mv.import_globals()
assert(rect_overlaps == mv.rect_overlaps and rect_contains == mv.rect_contains)
assert(collision == mv.collision and world_sweep == mv.world_sweep)
local bounds = {x = 0, y = 0, width = 10, height = 10}
assert(rect_overlaps(bounds, {x = 9, y = 9, width = 2, height = 2}))
assert(not rect_overlaps(bounds, {x = 10, y = 0, width = 1, height = 1}))
assert(rect_contains(bounds, 0, 0))
assert(not rect_contains(bounds, 10, 5) and not rect_contains(bounds, 5, 10))
local invalid_rects = {
  {x = 0, y = 0, width = 0, height = 1},
  {x = 0, y = 0, width = math.huge, height = 10},
  {x = 1e100, y = 0, width = 1, height = 1},
  {x = 0, width = 1, height = 1},
  {x = "0", y = 0, width = 1, height = 1},
  {x = 0, y = 0, width = 1, height = 1, extra = true},
  {x = 0, y = 0, width = 1, height = 1, ["x\0"] = 0},
}
for _, rect in ipairs(invalid_rects) do
  assert(not pcall(rect_overlaps, bounds, rect))
  assert(not pcall(rect_contains, rect, 0, 0))
end
assert(not pcall(rect_contains, bounds, 0/0, 0))
assert(not pcall(rect_contains, bounds, 0, "0"))
assert(not pcall(rect_overlaps, 0, 0, 10, 10, 9, 9, 2, 2))
assert(not pcall(rect_contains, 0, 0, 10, 10, 0, 0))
local collision = mv.collision
local circle = collision.circle {radius = 1}
local box = collision.rect {width = 4, height = 2}
local shifted = collision.circle {radius = 1, offset_x = 2}
local top_left = collision.rect {width = 4, height = 2, offset_x = 2, offset_y = 1}
assert(collision.overlap(circle, box) and collision.overlap(box, circle, {}))
assert(collision.overlap(top_left, circle, {a = {x = 10, y = 10}, b = {x = 10.5, y = 10.5}}))
assert(not collision.overlap(top_left, circle, {a = {x = 10, y = 10}, b = {x = 8, y = 10}}))
assert(collision.overlap(circle, circle, {b = {x = 1}}))
assert(not collision.overlap(circle, circle, {b = {x = 2}}))
assert(collision.overlap(circle, box, {a = {x = 2.5}}))
assert(not collision.overlap(circle, box, {a = {x = 2.8, y = 1.8}}))
assert(not collision.overlap(box, box, {b = {x = 3, angle = 90}}))
assert(collision.overlap(box, box, {b = {x = 2.5, angle = 90}}))
assert(collision.overlap(shifted, shifted, {a = {angle = 90}, b = {y = 4, angle = -90}}))
assert(collision.overlap(circle, circle, {b = {x = 2.5, scale = 2}}))
assert(not pcall(collision.circle, 1))
assert(not pcall(collision.rect, 1, 1))
assert(not pcall(collision.circle, {}))
assert(not pcall(collision.circle, {radius = "1"}))
assert(not pcall(collision.circle, {radius = 0}))
assert(not pcall(collision.rect, {width = 1}))
assert(not pcall(collision.rect, {width = 1, height = math.huge}))
assert(not pcall(collision.circle, {radius = 1, offset_x = 0/0}))
assert(not pcall(collision.circle, {radius = 1, unknown = 1}))
assert(not pcall(collision.overlap, circle, nil, box, nil))
assert(not pcall(collision.overlap, circle, box, nil))
assert(not pcall(collision.overlap, circle, box, {a = {scale = 0}}))
assert(not pcall(collision.overlap, circle, box, {a = {angle = math.huge}}))
assert(not pcall(collision.overlap, circle, box, {a = {z = 1}}))
assert(not pcall(collision.overlap, circle, box, {b = false}))
assert(not pcall(collision.overlap, circle, box, {b = {x = "2"}}))
assert(not pcall(collision.overlap, circle, box, {c = {}}))
assert(not pcall(function() circle.radius = 3 end))
local world = mv.world_create()
assert(mv.world_overlap(world, circle, nil) == nil)
assert(mv.world_add_rect(world, {x = 10, y = 0, width = 10, height = 10}) == 1)
assert(mv.world_add_rect(world, {x = 0, y = 0, width = 15, height = 10}) == 2)
assert(mv.world_overlap(world, circle, {x = 12, y = 5}) == 1)
assert(not pcall(mv.world_overlap, world, circle, {x = "12", y = 5}))
assert(mv.world_overlap(world, circle, {x = 2, y = 5}) == 2)
assert(mv.world_overlap(world, circle, {x = 21, y = 5}) == nil)
assert(not pcall(mv.world_add_rect, world, {x = 0, y = 0, width = 0, height = 1}))
assert(not pcall(mv.world_add_rect, world, {x = 0, y = 0, width = 1, height = 1, z = 0}))
assert(not pcall(mv.world_overlap, world, circle, {scale = 0}))
local mover = collision.rect {width = 2, height = 2}
local walls = mv.world_create()
mv.world_add_rect(walls, {x = 10, y = -10, width = 1, height = 20})
mv.world_add_rect(walls, {x = 10, y = -5, width = 1, height = 10})
local sweep = mv.world_sweep(walls, mover, {dx = 100, dy = 0})
assert(sweep.hit and sweep.id == 1 and math.abs(sweep.fraction - .09) < 1e-12)
assert(sweep.normal_x == -1 and sweep.normal_y == 0)
assert(not mv.world_sweep(walls, mover, {dx = 2, dy = 0}).hit)
assert(not mv.world_sweep(walls, mover, {x = 9, dx = -2, dy = 0}).hit)
assert(mv.world_sweep(walls, mover, {x = 9, dx = 2, dy = 0}).fraction == 0)
assert(not pcall(mv.world_sweep, walls, mover, {x = 10, dx = 1, dy = 0}))
local round = mv.world_sweep(walls, circle, {dx = 100, dy = 0})
assert(round.hit and math.abs(round.fraction - .09) < 1e-12)
assert(round.normal_x == -1 and round.normal_y == 0)
assert(not mv.world_sweep(walls, circle, {y = 11, dx = 100, dy = 0}).hit)
local corner = mv.world_sweep(walls, circle, {y = 10.5, dx = 100, dy = 0})
assert(corner.hit and math.abs(corner.fraction - (10 - math.sqrt(.75)) / 100) < 1e-12)
local rotated = mv.world_sweep(walls, mover, {angle = 45, scale = 2, dx = 100, dy = 0})
assert(rotated.hit and rotated.id == 1 and rotated.normal_x == -1 and rotated.normal_y == 0)
assert(math.abs(rotated.fraction - (10 - 2 * math.sqrt(2)) / 100) < 1e-12)
local clear = mv.world_sweep(walls, mover, {dx = 0, dy = 0})
assert(not clear.hit and clear.fraction == 1 and clear.id == nil)
assert(clear.normal_x == 0 and clear.normal_y == 0)
assert(not pcall(mv.world_sweep, walls, mover, nil, 1, 0))
assert(not pcall(mv.world_sweep, walls, mover))
assert(not pcall(mv.world_sweep, walls, mover, {dx = 1}))
assert(not pcall(mv.world_sweep, walls, mover, {dy = 0}))
assert(not pcall(mv.world_sweep, walls, mover, {dx = "1", dy = 0}))
assert(not pcall(mv.world_sweep, walls, mover, {dx = 0/0, dy = 0}))
assert(not pcall(mv.world_sweep, walls, mover, {dx = 1e308, dy = 0}))
assert(not pcall(mv.world_sweep, walls, mover, {dx = 1, dy = 0, scale = 0}))
assert(not pcall(mv.world_sweep, walls, mover, {dx = 1, dy = 0, extra = true}))
do
  local temporary = mv.world_create()
  mv.world_add_rect(temporary, {x = 0, y = 0, width = 1, height = 1})
end
collectgarbage("collect")
assert(mv.world_overlap(world, circle, {x = 12, y = 5}) == 1)
local task = mv.process(function(self)
  assert(self.collider == circle)
end, {defaults = {collider = circle}})
mv.start(function() task(); task() end)
