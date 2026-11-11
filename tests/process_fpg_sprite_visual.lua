-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local selected
local actor = mv.process(function(self)
  assert(self.graph == selected, "unchanged sprite identity")
  while true do mv.frame() end
end)
mv.start(function()
  local library = mv.load_asset("tests/fixtures/fpg/bennugd-8.fpg")
  selected = mv.graph(library, 7)
  actor {x = 10, y = 10, graph = selected}
  collectgarbage("collect")
end, {width = 64, height = 64, title = "Native Lua FPG sprite ownership"})
