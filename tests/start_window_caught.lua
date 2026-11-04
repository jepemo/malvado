-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local ok, message = pcall(mv.start, function() error("unexpected initializer") end)
assert(not ok and message:find("could not open window", 1, true))
