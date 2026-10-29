local mv = require("malvado")
local P = mv.process(function(self)
    assert(type(self) == "userdata")
    assert(self.id > 0 and self.parent == nil and self.priority == 0)
    assert(self.status == "running")
    self.value = 2
    assert(self.value == 2)
    for _, field in ipairs({"id", "parent", "priority", "status", "recv"}) do
        assert(not pcall(function() self[field] = 5 end))
    end
    assert(not pcall(rawset, self, "id", 5))
    mv.set_priority(self.id, 8)
    assert(self.priority == 8)
end)
mv.start(function() P({id = -1, parent = -1, priority = -1}) end)
