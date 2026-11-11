-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local red = mv.graphics.rect(20, 20, {color = {255, 0, 0}})
local green = mv.graphics.rect(20, 20, {color = {0, 255, 0}})
local actor = mv.process(function(self)
  while true do
    -- The native frame hook changes -1; this round trip proves shared storage.
    if self.z == -1 then self.z = -2 end
    mv.frame()
  end
end)
mv.start(function()
  actor {graph = red, x = 20, y = 20, z = 10}
  actor {graph = green, x = 20, y = 20}
end, {width = 64, height = 64, title = "Native process order"})
