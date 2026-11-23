-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local actor = mv.process(function(self)
  while true do mv.frame() end
end)
mv.start(function()
  actor {x = 20, y = 20, graph = mv.graphics.rect {width = 4, height = 4, color = {255, 0, 0}}}
end, {width = 64, height = 64, title = "Native process view"})
