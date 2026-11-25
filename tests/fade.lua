-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
mv.import_globals()
assert(fade_to == mv.fade_to and fade_off == mv.fade_off and fade_on == mv.fade_on)
assert(fade_active == mv.fade_active and fade_color == mv.fade_color)

local black = {0, 0, 0, 255}
local maximum_ms = 1000000 * 1000 / 60

local function rejected(fn, ...)
  local before = fade_color()
  local active = fade_active()
  assert(not pcall(fn, ...))
  assert(fade_active() == active)
  local after = fade_color()
  for i = 1, 4 do assert(before[i] == after[i]) end
end

local function expect_duration(ticks, fn, ...)
  assert(select("#", fn(...)) == 0)
  for tick = 0, ticks do
    assert(fade_active() == (tick < ticks), ("fade timing at tick %d of %d"):format(tick, ticks))
    if tick < ticks then frame() end
  end
end

local request_tick
local requester = process(function(self)
  assert(self.fps == 15)
  self.fps = 10
  request_tick = snapshot().tick
  fade_off(100)
  if self.pause_signal then
    signal(self, self.pause_signal)
    frame()
    error("paused fade requester resumed")
  end
end)

local task = mv.process(function()
  assert(not fade_active() and fade_color()[4] == 0)
  local boundary_ms = 1000 / 60
  local cases = {
    {0, 0}, {0x1p-1074, 1}, {1, 1}, {0.125, 1},
    {boundary_ms - 2^-48, 1}, {boundary_ms, 1}, {boundary_ms + 2^-48, 2},
    {100 - 2^-46, 6}, {100, 6}, {100 + 2^-46, 7},
    {250, 15}, {500, 30}, {1000, 60},
  }
  for _, case in ipairs(cases) do expect_duration(case[2], fade_off, case[1]) end
  expect_duration(15, fade_off)
  expect_duration(15, fade_off, nil)
  expect_duration(15, fade_on)
  expect_duration(15, fade_on, nil)
  expect_duration(15, fade_to, {color = BLUE})
  expect_duration(15, fade_to, {color = BLUE, duration_ms = nil})

  for _, invalid in ipairs({-1, -0.125, 0/0, math.huge, -math.huge, "5", true, {},
      maximum_ms + 2^-29, 1e300}) do
    rejected(fade_off, invalid)
    rejected(fade_on, invalid)
    rejected(fade_to, {color = black, duration_ms = invalid})
  end
  rejected(fade_to)
  rejected(fade_to, false)
  rejected(fade_to, {duration_ms = 0})
  rejected(fade_to, {color = black, ticks = 0})
  rejected(fade_to, {color = black, extra = 0})
  rejected(fade_to, {color = black, [1] = 0})
  rejected(fade_to, {color = {0, 0, 0, 256}, duration_ms = 0})
  rejected(fade_to, {color = {0, 0}, duration_ms = 0})
  rejected(fade_to, {color = black}, nil)
  rejected(fade_off, 0, nil)
  rejected(fade_on, nil, nil)
  rejected(fade_active, nil)
  rejected(fade_color, nil)

  fade_on(0)
  mv.fade_off(30)
  mv.fade_on(30)
  rejected(fade_to, {color = black, duration_ms = -1})
  local expected = {0, 127, 255, 128, 0}
  for i = 1, #expected do
    local alpha = mv.fade_color()[4]
    assert(alpha == expected[i], string.format("step %d: %d", i, alpha))
    assert(mv.fade_active() == (i < #expected))
    if i < #expected then mv.frame() end
  end
  mv.fade_to {color = {10, 20, 30, 40}, duration_ms = 0}
  local color = mv.fade_color()
  assert(color[1] == 10 and color[2] == 20 and color[3] == 30 and color[4] == 40)
  mv.fade_off(1)
  for _ = 1, 16 do mv.fade_on(0) end
  local ok, message = pcall(mv.fade_to, {color = BLUE, duration_ms = 0})
  assert(not ok and message:find("fade queue is full", 1, true))
  assert(fade_color()[4] == 40)
  frame()
  assert(not fade_active() and fade_color()[4] == 0)

  set_fps(15)
  for _, scenario in ipairs({
    {SIGNAL_SLEEP, STATUS_SLEEPING}, {SIGNAL_FREEZE, STATUS_FROZEN}, {nil, STATUS_DEAD},
  }) do
    fade_on(0)
    request_tick = nil
    local child = requester {pause_signal = scenario[1]}
    frame()
    if not request_tick then frame() end
    assert(request_tick and snapshot().tick - request_tick == 1)
    for tick = 1, 6 do
      assert(child.status == scenario[2])
      assert(fade_color()[4] == 255 * tick // 6)
      assert(fade_active() == (tick < 6))
      if tick < 6 then frame() end
    end
    if alive(child) then kill(child) end
  end

  fade_on(0)
  fade_off(maximum_ms)
  assert(fade_active() and fade_color()[4] == 0)
  rejected(fade_on, maximum_ms + 2^-29)
  frame()
  assert(fade_active() and fade_color()[4] == 0)
end)
mv.start(function() task() end)
