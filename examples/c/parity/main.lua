-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

require("malvado").import_globals()
local worker = process(function(self)
    local guard <close> = setmetatable({}, {
        __close = function() print("worker: closed") end,
    })
    assert(self:recv() == "ping")
    print("worker: ping")
    frame()
end)
local controller = process(function(self)
    send(self.worker, "ping")
    signal(self.worker, "freeze")
    print("controller: freeze")
    frame()
    signal(self.worker, "wakeup")
    print("controller: wake")
    frame()
    kill(self.worker)
    print("controller: kill")
end)
start(function()
    local id = worker()
    local control = controller({worker = id})
    set_priority(control, 10)
end)
