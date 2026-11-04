-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
assert(mv.rect_overlaps(0, 0, 10, 10, 9, 9, 2, 2))
assert(not mv.rect_overlaps(0, 0, 10, 10, 10, 0, 1, 1))
assert(mv.rect_contains(0, 0, 10, 10, 0, 0))
assert(not mv.rect_contains(0, 0, 10, 10, 10, 5))
assert(not pcall(mv.rect_overlaps, 0, 0, 10, 10, 0, 0, 0, 1))
assert(not pcall(mv.rect_contains, 0, 0, 10, 10, 0/0, 0))
assert(not pcall(mv.rect_contains, 0, 0, math.huge, 10, 0, 0))
mv.start(function() end)
