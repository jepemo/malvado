-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local library = mv.load_asset("tests/fixtures/fpg/bennugd-8.fpg")
local definition = mv.animation(library, {7,42}, {fps = 20})
local elapsed = mv.graphics.animation_elapsed
local expected = {}
for line in io.lines("tests/fixtures/process-animation.txt") do
  expected[#expected + 1] = assert(tonumber(line:match("^(%d+)")))
end
local Actor = mv.process(function(self)
  for index = 1, #expected - 1 do
    assert(math.abs(elapsed(self) * 1000 - expected[index]) < 0.001)
    mv.frame()
  end
  assert(elapsed(self) == nil) -- The C host cleared the shared clock.
  self.graph = self.graph
  assert(elapsed(self) == 0)
  mv.frame()
end)
mv.start(function() Actor {graph = definition} end)
