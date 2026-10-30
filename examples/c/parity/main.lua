-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local worker = mv.process(function(self)
    local guard <close> = setmetatable({}, {
        __close = function() print("worker: closed") end,
    })
    assert(self:recv() == "ping")
    print("worker: ping")
    mv.frame()
end)
local controller = mv.process(function(self)
    mv.send(self.worker, "ping")
    mv.signal(self.worker, "freeze")
    print("controller: freeze")
    mv.frame()
    mv.signal(self.worker, "wakeup")
    print("controller: wake")
    mv.frame()
    mv.kill(self.worker)
    print("controller: kill")
end)
mv.start(function()
    local id = worker()
    local control = controller({worker = id})
    mv.set_priority(control, 10)
end)
