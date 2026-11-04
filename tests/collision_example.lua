-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local original_process, original_frame, original_text = mv.process, mv.frame, mv.draw_text
local ticks, definitions, player, player_thread = 0, 0, nil, nil
local messages = {}
mv.process = setmetatable({collision = original_process.collision}, {
  __call = function(_, fn, options)
    definitions = definitions + 1
    local is_player = definitions == 2
    return original_process(function(self)
      if is_player then player, player_thread = self, coroutine.running() end
      return fn(self)
    end, options)
  end
})
mv.key = function(name)
  if name == "escape" then return ticks >= 140 end
  if name == "right" then return ticks < 75 end
  if name == "left" then return ticks >= 75 end
  return false
end
mv.draw_text = function(text, ...)
  if text == "Collision!" then messages[ticks] = true end
  return original_text(text, ...)
end
mv.frame = function()
  if coroutine.running() == player_thread then
    if ticks == 139 then
      assert(player.x == 160)
      assert(not messages[64] and messages[65] and messages[83] and not messages[84])
    end
    ticks = ticks + 1
  end
  return original_frame()
end
return dofile("examples/collision/main.lua")
