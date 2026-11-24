-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
mv.import_globals()
assert(set_fps == mv.set_fps)

local function expect_error(message, fn, ...)
  local ok, err = pcall(fn, ...)
  assert(not ok and err:find(message, 1, true), tostring(err))
end

expect_error("requires a running game", set_fps, 30)

local function check_cadence(self)
  assert(self.fps == self.expected_fps)
  local previous
  for i, expected_tick in ipairs(self.ticks) do
    local tick = snapshot().tick
    assert(tick == expected_tick, ("fps %d: expected tick %d, got %d"):format(self.fps, expected_tick, tick))
    local delta = previous and (tick - previous) / 60 or 0
    assert(math.abs(self.delta - delta) < 1e-12)
    previous = tick
    if i < #self.ticks then frame() end
  end
end

local worker = process(check_cadence)
local defaults = {fps = 20}
local defined = process.define(check_cadence, {defaults = defaults})
local sleeping, frozen

local producer = process(function(self)
  assert(self.fps == 60)
  frame()
  -- The two paused workers have each accumulated half of an activation.
  signal(sleeping, SIGNAL_SLEEP)
  signal(frozen, SIGNAL_FREEZE)
  mv.set_fps(15)
  frame()
  -- The other process has already spawned using the new default.
  set_fps(60)
  local child = spawn(worker, {expected_fps = 60, ticks = {4, 5, 6}})
  assert(child.fps == 60 and child.parent == self)
  frame()
  frame()
  assert(sleeping.status == "sleeping" and frozen.status == "frozen")
  assert(sleeping.fps == 30 and frozen.fps == 30)
  signal(sleeping, SIGNAL_WAKEUP)
  signal(frozen, SIGNAL_WAKEUP)
  -- Keep the attached child alive through its last activation.
  frame()
  frame()
end)

local consumer = process(function()
  frame()
  local child = mv.process.spawn(worker, {expected_fps = 15, ticks = {3, 7, 11}}, {attached = false})
  assert(child.fps == 15 and child.parent == nil)
end)

local adjustable = process(function(self)
  assert(self.fps == 30 and snapshot().tick == 1)
  self.fps = 60
  frame()
  assert(self.fps == 60 and snapshot().tick == 2)
end)

local before_start = worker {expected_fps = 0, ticks = {1, 2}}
assert(before_start.fps == 0)
start(function()
  assert(before_start.fps == 0)
  local fields = {expected_fps = 30, ticks = {1, 3, 5, 7}}
  assert(worker(fields).fps == 30 and fields.fps == nil)
  sleeping = worker {expected_fps = 30, ticks = {1, 6, 8}}
  frozen = worker {expected_fps = 30, ticks = {1, 6, 8}}
  assert(defined {expected_fps = 20, ticks = {1, 4, 7}}.fps == 20)
  assert(defined {fps = 0, expected_fps = 0, ticks = {1, 2}}.fps == 0)
  assert(defaults.fps == 20)
  adjustable()

  expect_error("fps must be an integer", mv.set_fps)
  expect_error("fps must be an integer", set_fps, nil)
  for _, invalid in ipairs({false, "15", 1.5, 15.0, -1, 61, math.huge}) do
    expect_error("fps must be an integer", set_fps, invalid)
  end
  expect_error("fps must be an integer", mv.spawn, worker, {fps = 61})
  assert(worker {expected_fps = 30, ticks = {1, 3}}.fps == 30)

  set_fps(0)
  assert(worker {expected_fps = 0, ticks = {1, 2}}.fps == 0)
  mv.set_fps(60)
  producer()
  consumer()
end, {fps = 30, width = 64, height = 64})
expect_error("requires a running game", set_fps, 15)
