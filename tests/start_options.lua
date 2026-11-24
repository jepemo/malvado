-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local function resize_error(parameter, ...)
  local ok, message = pcall(mv.set_window_size, ...)
  assert(not ok and message:find(parameter, 1, true), message)
end
resize_error("requires a running game", 320, 240)
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
local function expect_size(width, height)
  local w, h = mv.window_size()
  assert(w == width and h == height)
end
local calls = 0
local Resize = mv.process(function()
  expect_size(360, 270)
  mv.set_window_size(640, 480)
  mv.frame()
  expect_size(640, 480)
  calls = calls + 1
end)
mv.start(function()
  expect_size(320, 240)
  resize_error("width")
  resize_error("height", 320)
  resize_error("width", nil, 240)
  resize_error("height", 320, nil)
  for _, invalid in ipairs {true, false, "320", 1.5, 1.0, math.huge, 0/0, -1, 0, 2147483648} do
    resize_error("width", invalid, 240)
    resize_error("height", 320, invalid)
    expect_size(320, 240)
  end
  mv.set_window_size(1, 2147483647)
  expect_size(1, 2147483647)
  mv.set_window_size(360, 270)
  mv.set_window_size(360, 270)
  expect_size(360, 270)
  Resize()
end, {width = 320, height = 240})
assert(calls == 1)
resize_error("requires a running game", 320, 240)
expect_size(640, 480)
assert(not pcall(mv.start, init))
