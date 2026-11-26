-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")

local Mover = mv.process(function(self)
    local speed = 120 -- Logical pixels per second.
    local elapsed = 0
    while true do
        local dt = self.delta
        elapsed = elapsed + dt
        self.x = self.x + speed * dt
        if elapsed + 1e-9 >= 0.1 then
            print(string.format("%d fps: %.2f px after %.2f s", self.fps, self.x, elapsed))
            return
        end
        mv.frame()
    end
end)

return {
    init = function()
        Mover {fps = 60}
        Mover {fps = 30}
    end,
}
