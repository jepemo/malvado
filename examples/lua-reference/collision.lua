-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")

local zone = {x = 10, y = 10, width = 16, height = 16}
local neighbor = {x = 20, y = 20, width = 8, height = 8}
assert(mv.rect_overlaps(zone, neighbor))
assert(mv.rect_contains(zone, 12, 12))
assert(not mv.rect_contains(zone, 26, 12))
print("Rectangle tests passed")

local circle = mv.collision.circle {radius = 8}
local box = mv.collision.rect {width = 20, height = 12}
assert(mv.collision.overlap(circle, box, {a = {x = 12}}))
assert(not mv.collision.overlap(circle, box, {a = {x = 40}}))
print("Shape tests passed")

local target = mv.process(function()
    mv.frame()
end)

local probe = mv.process(function(self)
    local first = mv.collision(target)
    local all = self:collisions(target)
    assert(first and #all == 1 and first == all[1])
    assert(self:collision(first) == first)
    print("Process collision found")
end)

mv.start(function()
    target {x = 10, collider = box}
    probe {x = 12, collider = circle}
end)
