-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")

local Enemy = mv.process(function(self)
    while self.x > 0 do
        self.x = self.x - 1
        mv.frame()
    end
    print("Enemy reached the left edge")
end)

local Bullet = mv.process(function(self)
    while self.x < 6 do
        self.x = self.x + 2
        mv.frame()
    end
    print("Bullet reached the right edge")
end)

local Wave = mv.process(function()
    local enemy = Enemy {x = 6}
    mv.frame()
    mv.frame()
    local bullet = Bullet {x = 0}
    -- Attached actors need their owner to stay alive until they finish.
    while mv.process.alive(enemy) or mv.process.alive(bullet) do
        mv.frame()
    end
    print("Wave clear")
end)

return {init = function() Wave() end}
