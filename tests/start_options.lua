-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local init = function() end
assert(not pcall(mv.start, init, true))
assert(not pcall(mv.start, init, "options"))
assert(not pcall(mv.start, init, {width = 0}))
assert(not pcall(mv.start, init, {width = 4294967297}))
assert(not pcall(mv.start, init, {height = 4294967297}))
assert(not pcall(mv.start, init, {width = "320"}))
assert(not pcall(mv.start, init, {title = 5}))
assert(not pcall(mv.start, init, {title = "a\0b"}))
assert(not pcall(mv.start, init, {unknown = true}))
assert(not pcall(mv.start, init, setmetatable({}, {})))
local function unexpected_init() error("unexpected initializer") end
for _, value in ipairs({false, "30", 30.5, -1, 61}) do
  local ok, err = pcall(mv.start, unexpected_init, {fps = value})
  assert(not ok and err:find("game fps must be an integer", 1, true))
end
local called = false
mv.start(function() called = true end, {width = 320, height = 240})
assert(called)
assert(not pcall(mv.start, init))
