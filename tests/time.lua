-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local before = mv.time_seconds()
assert(type(before) == "number")
local task = mv.process(function()
  local first = mv.time_seconds()
  mv.frame()
  assert(mv.time_seconds() >= first)
end)
mv.start(function() task {} end)
assert(mv.time_seconds() >= before)
