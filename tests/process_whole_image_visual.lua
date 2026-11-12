-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local selected
local actor = mv.process(function(self)
  assert(self.graph == selected, "unchanged whole-image identity")
  local width, height = mv.image_size(self.graph)
  assert(width == 3 and height == 5)
  while true do mv.frame() end
end)
mv.start(function()
  selected = mv.load_asset("tests/fixtures/odd-red.png", {pivot = "center"})
  actor {x = 10, y = 10, graph = selected}
  collectgarbage("collect")
end, {width = 64, height = 64, title = "Native Lua fractional whole image"})
