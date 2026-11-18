-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")

local worker = mv.process(function(self)
    print(self.name .. ": 1")
    mv.frame() -- Let the other worker act before this one resumes.
    print(self.name .. ": 2")
end)

return {
    init = function()
        worker({name = "first"})
        worker({name = "second"})
    end,
}
