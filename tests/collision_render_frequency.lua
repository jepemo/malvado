-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local proc = mv.process
local shape = mv.graphics.circle(2)
local Target = proc(function() while true do proc.frame() end end,
  {defaults = {graph = shape}})
local target
local Player = proc(function(self)
  for _, x in ipairs({10, 3, 0, 10}) do
    self.x = x
    assert((proc.collision(self, target) ~= nil) == (x < 4))
    proc.frame()
  end
  mv.kill(target)
end, {defaults = {graph = shape}})

return {width = 80, height = 60, init = function()
  target = Target({x = 0, y = 20})
  Player({x = 10, y = 20})
end}
