-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local count = 0
local actor = mv.process(function()
  for _ = 1, 7 do
    count = count + 1
    mv.frame()
  end
  count = count + 1
end)
mv.start(function() actor() end, {width=160,height=120,title="Shared runner clock"})
assert(count == 8)
