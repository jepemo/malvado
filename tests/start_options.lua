-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local init = function() end
assert(not pcall(mv.start, init, true))
assert(not pcall(mv.start, init, "options"))
assert(not pcall(mv.start, init, {width = 0}))
assert(not pcall(mv.start, init, {width = 4294967297}))
assert(not pcall(mv.start, init, {height = 4294967297}))
local called = false
mv.start(function() called = true end, {width = 320, height = 240})
assert(called)
assert(not pcall(mv.start, init))
