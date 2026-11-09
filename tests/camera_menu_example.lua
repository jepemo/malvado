-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local define, original_frame = mv.process, mv.frame
local definitions, ticks, inventory_thread, inventory = 0, 0
mv.process = function(fn, options)
  definitions = definitions + 1
  local selected = definitions == 2
  return define(function(self)
    if selected then inventory_thread, inventory = coroutine.running(), self end
    return fn(self)
  end, options)
end
mv.key = function(name)
  if name == "escape" then return ticks >= 12 end
  return name == "right"
end
mv.frame = function()
  if coroutine.running() == inventory_thread then
    ticks = ticks + 1
    assert(inventory.x == 0 and inventory.y == 0 and inventory.camera == false)
    assert(inventory.scroll and inventory.region)
    local x, y = mv.camera.position()
    assert(x == 200 and y == 100)
  end
  return original_frame()
end
return dofile("examples/camera-menu/main.lua")
