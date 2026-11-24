-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

require("malvado").import_globals()

-- Every meteor is an independent process with its own position and speed.
local Meteor = process(function(self)
  while not key(KEY_ESCAPE) do
    self.y = self.y + self.speed * self.delta
    if self.y > 360 then self.y = -16 end
    frame()
  end
end, {defaults = {graph = circle {radius = 12, color = ORANGE}}})

start(function()
  Meteor({x = 100, y = 20, speed = 80})
  Meteor({x = 320, y = 80, speed = 140})
  Meteor({x = 540, y = 40, speed = 200})
end, {title = "Meteors", width = 640, height = 360})
