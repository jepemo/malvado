-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")

local world = mv.world_create()
local wall = world:add_rect {x = 100, y = 0, width = 10, height = 80}
local shape = mv.collision.circle {radius = 8}
assert(world:overlap(shape, {x = 96, y = 40}) == wall)

local sweep = world:sweep(shape, {x = 0, y = 40, dx = 200, dy = 0})
assert(sweep.hit and sweep.id == wall)
print(("First contact at %.0f pixels"):format(200 * sweep.fraction))

local player = mv.process(function(self)
    assert(mv.world_collision {x = 96} == wall)
    assert(self.x == 0)
    local result = world:move(self, 200, 0)
    assert(result.hit and result.id == wall)
    assert(world:collision(self) == nil)
    print(("Player moved %.0f pixels and stopped at the wall"):format(result.moved_x))
end)

mv.start(function()
    player {x = 0, y = 40, world = world, collider = shape}
end)
