-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local args = { value = 1 }
local seen = {}
local closed = 0
local P = mv.process(function(self)
  local cleanup <close> = setmetatable({}, { __close = function() closed = closed + 1 end })
  seen[#seen + 1] = self.value
  self.value = self.value + 1
  local co = coroutine.create(function() mv.frame() end)
  local ok = coroutine.resume(co)
  assert(not ok, "frame from nested coroutine must fail")
  mv.frame()
  seen[#seen + 1] = self.value
end)
mv.start(function()
  P(args)
  P(args)
end)
assert(table.concat(seen, ",") == "1,1,2,2")
assert(args.value == 1 and args.id == nil)
assert(closed == 2)
