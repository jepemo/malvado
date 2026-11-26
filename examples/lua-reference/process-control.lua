-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")

local worker = mv.process.define(function(self)
    print(self.name .. " woke up")
    while true do mv.frame() end
end, {defaults = {name = "Worker"}})

local manager = mv.process(function(self)
    local guard = worker {name = "Guard"}
    local scout = mv.spawn(worker, {name = "Scout"}, {attached = false})
    assert(#mv.find(worker) == 2)
    assert(self:children()[1] == guard)
    assert(#self:descendants {relation = "creation", type = worker} == 2)
    assert(mv.snapshot().tick >= 1)
    print("Found two workers, one attached")

    guard:set_priority(10)
    assert(mv.get_priority(guard) == 10)
    scout:signal(mv.SIGNAL_SLEEP)
    self:signal_tree(mv.SIGNAL_FREEZE, {relation = "creation"})
    assert(mv.status(scout) == mv.STATUS_FROZEN)
    self:signal_tree(mv.SIGNAL_WAKEUP, {relation = "creation"})
    mv.frame()

    assert(guard:kill())
    assert(not guard:alive())
    -- The creation tree still contains the detached scout.
    self:kill_tree()
end)

local controller = mv.process(function()
    local level = manager()
    while level:alive() do mv.frame() end
    assert(level.status == mv.STATUS_DEAD)
    assert(#mv.find(worker) == 0)
    print("Level and workers stopped")
end)

mv.start(function() controller() end)
