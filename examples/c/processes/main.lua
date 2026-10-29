-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local counter = mv.process(function(self)
    while self.remaining > 0 do
        print("tick: " .. self.remaining)
        self.remaining = self.remaining - 1
        if self.remaining > 0 then mv.frame() end
    end
end)
mv.start(function() counter({remaining = 3}) end)
