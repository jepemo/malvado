-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")

local Door = mv.process(function(self)
    while true do
        if mv.recv() == "open" then
            print("Door opened")
            mv.send(self.controller, "opened")
            return
        end
        mv.frame()
    end
end)

local Controller = mv.process(function(self)
    local door = Door {controller = self}
    mv.signal(door, mv.SIGNAL_FREEZE)
    mv.send(door, "open")
    mv.frame()
    assert(door.status == mv.STATUS_FROZEN and mv.recv() == nil)
    print("Open request queued")

    mv.signal(door, mv.SIGNAL_WAKEUP)
    while mv.recv() ~= "opened" do mv.frame() end
    print("Exit ready")
end)

return {init = function() Controller() end}
