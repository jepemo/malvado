-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local main
main = mv.process(function(self)
  self.graph = mv.graphics.circle(5)
  self.x = 0
  mv.frame()
  local native = mv.find(main)[2]
  assert(native and mv.process.collision(self, main) == native)
  assert(mv.process.collision(self, native) == native)
  assert(mv.process.collisions(self, main)[1] == native)
  self.x = 100
  assert(mv.process.collision(self, native) == nil)
  mv.frame()
end)
mv.start(function() main() end)
