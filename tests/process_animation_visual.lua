-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local definition
local Actor = mv.process(function(self)
  local elapsed = mv.animation_elapsed
  while true do
    assert(mv.animation_finished() == (elapsed() == 100))
    assert(self:animation_finished() == mv.graphics.animation_finished(self))
    assert(self.graph == definition, "native playback preserves Lua identity")
    self.graph = definition -- Same assignment must not restart or copy frames.
    assert(mv.animation_finished(self) == (elapsed(self) == 100))
    collectgarbage("collect")
    mv.frame()
  end
end)
mv.start(function()
  local library = mv.load_asset("fixtures/fpg/bennugd-8.fpg")
  definition = mv.animation(library, {7,42}, {fps=20, loop=false})
  Actor {x=10, y=10, graph=definition, fps=5}
end, {width=64,height=64,title="Lua native visual animation"})
