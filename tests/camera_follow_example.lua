-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local define, original_frame = mv.process, mv.frame
local player_handle
local definitions, ticks, player_thread, last_x = 0, 0
mv.process = function(fn, options)
  definitions = definitions + 1
  local player = definitions == 2
  return define(function(self)
    if player then player_thread, player_handle = coroutine.running(), self end
    return fn(self)
  end, options)
end
mv.key = function(name)
  if name == "escape" then return ticks >= 12 end
  return name == "right"
end
mv.frame = function()
  if coroutine.running() == player_thread then
    ticks = ticks + 1
    -- Camera state from the previous completed tick remains independent of
    -- the current process update. The pixel check verifies the new projection.
    local x, y = mv.camera.position()
    assert(x == 3 * (ticks - 1) and y == 0)
    assert(player_handle.x == 320 + ticks * 3 and player_handle.y == 240)
    last_x = player_handle.x
  end
  return original_frame()
end
dofile("examples/camera-follow/main.lua")
assert(ticks == 12 and last_x == 356)
