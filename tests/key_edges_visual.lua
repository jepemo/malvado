-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local calls = 0
local observed = {}
local Fast = mv.process(function()
  for tick = 1, 5 do
    assert(mv.snapshot().tick == tick)
    local pressed = tick == 2 or tick == 5
    local released = tick == 4 or tick == 5
    if not observed[tick] then
      print(string.format("lua-key: %d %d %d", tick, pressed and 1 or 0, released and 1 or 0))
      observed[tick] = true
    end
    assert(mv.key_pressed("space") == pressed)
    assert(mv.input.key_pressed("space") == pressed) -- read does not consume
    assert(mv.key_released("space") == released)
    assert(mv.input.key_released("space") == released)
    if tick < 5 then mv.frame() end
  end
  calls = calls + 1
end)
local Slow = mv.process(function()
  for _, tick in ipairs({1, 3, 5}) do
    assert(mv.snapshot().tick == tick)
    assert(mv.key_pressed("space") == (tick == 5))
    assert(mv.key_released("space") == (tick == 5))
    if tick < 5 then mv.frame() end
  end
  calls = calls + 1
end)
mv.start(function() Fast(); Fast(); Slow {fps = 30} end,
  {width = 160, height = 120, title = "Keyboard tick snapshots"})
assert(calls == 3)
