local mv = require("malvado")
local saved
local weak = setmetatable({}, {__mode = "v"})
local P = mv.process(function(self)
    assert(type(self) == "userdata")
    assert(self.id > 0 and self.parent == nil and self.priority == 0)
    assert(self.status == "running")
    assert(self.x == 0 and self.y == 0 and self.z == 0 and self.angle == 0)
    assert(self.size == 1 and self.width == 0 and self.height == 0 and self.fps == 0)
    saved = self
    self.resource = {}
    weak[1] = self.resource
    self.value = 2
    assert(self.value == 2)
    assert(self.delta == 0)
    for _, field in ipairs({"id", "parent", "creator_id", "priority", "status", "reason", "recv", "delta"}) do
        assert(not pcall(function() self[field] = 5 end))
    end
    assert(not pcall(rawset, self, "id", 5))
    mv.set_priority(self, 8)
    assert(self.priority == 8)
end)
mv.start(function()
    local fields = {"id", "parent", "creator_id", "priority", "status", "reason", "recv", "delta"}
    for _, name in ipairs(fields) do
        local input = {[name] = false}
        assert(not pcall(P, input))
        local defaults = mv.process(function() error("must not run") end, {defaults = input})
        assert(not pcall(defaults, {}))
    end
    assert(not pcall(P, {children_attached = 1}))
    assert(#mv.snapshot().processes == 0)
    local first = P({})
    assert(first.id == 1)
end)

assert(saved.status == "dead" and saved.reason == "returned" and saved.id > 0)
assert(not pcall(function() return saved.value end))
assert(not pcall(function() saved.value = 3 end))
assert(not pcall(function() return saved.parent end))
collectgarbage("collect")
assert(weak[1] == nil)
