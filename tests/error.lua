-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local bad = mv.process(function() error("expected failure") end)
mv.start(function() bad {} end)
