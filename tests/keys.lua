-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local process = mv.process(function()
    assert(not mv.key("space"))
    assert(not mv.key("escape"))
    assert(not pcall(mv.key, "unknown"))
end)
mv.start(function() process() end)
