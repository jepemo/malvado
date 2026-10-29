-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local trace = {}
local child = mv.process(function(self)
  trace[#trace + 1] = "child:" .. self.value
  mv.frame()
  trace[#trace + 1] = "child:end"
end)
local parent = mv.process(function(self)
  trace[#trace + 1] = "parent:start"
  child { value = 42 }
  mv.frame()
  trace[#trace + 1] = "parent:end"
end)
mv.start(function() parent {} end)
assert(table.concat(trace, ",") == "parent:start,parent:end,child:42,child:end", table.concat(trace, ","))
local ok = pcall(mv.frame)
assert(not ok, "frame outside a process must fail")
