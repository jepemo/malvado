-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local p = mv.process
local shape = mv.graphics.circle(2)
local Worker = p(function() end, {defaults = {graph = shape}})
local a, b = Worker(), Worker()
assert(p.collision(a, Worker) == b)
test_arm()
local hit = p.collision(a, Worker)
test_disarm()
assert(hit == b)
mv.kill(a)
mv.kill(b)
mv.start(function() end)
test_report(true)
