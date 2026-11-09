-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local define, frame = mv.process, mv.frame
local frames, seen, unique = 0, {}, 0
local actor
mv.process = function(fn, options)
  return define(function(self)
    actor = self
    return fn(self)
  end, options)
end
mv.key = function(name) return name == "escape" and frames >= 8 end
mv.frame = function()
  local image = assert(actor.graph)
  if not seen[image] then
    seen[image] = true
    unique = unique + 1
  end
  assert(actor.fps == 15 and actor.x == 360 and actor.y == 260)
  frames = frames + 1
  return frame()
end

dofile("examples/sprites/main.lua")
assert(frames >= 8 and unique == 6)
