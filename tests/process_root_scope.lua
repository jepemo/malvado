-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local proc = mv.process
local seen = 0
local behavior = function() end
assert(not pcall(proc.define, behavior, {child_attached = false}))
assert(not pcall(proc.define, behavior, {defaults = 1}))
assert(not pcall(proc.define, behavior, {children_attached = "false"}))
assert(not pcall(proc.define, behavior, {[1] = true}))
assert(not pcall(proc.define, behavior, {["defaults\0x"] = {}}))
local Root = proc.define(function(self)
  assert(self.parent == nil and self.creator_id == nil)
  seen = seen + 1
end)
local early = Root {}
assert(mv.snapshot().processes[1].type == 1)
assert(not pcall(proc.spawn, Root, {}, {attachedd = false}))
assert(not pcall(proc.spawn, Root, {}, {attached = "false"}))
assert(not pcall(proc.spawn, Root, {}, {[1] = false}))
assert(early.parent == nil and early.creator_id == nil)
local ok, message = pcall(proc.spawn, Root, {}, {attached = true})
assert(not ok and message:find("without an owner", 1, true))
local other = proc.spawn(Root, {}, {attached = false})
assert(other.parent == nil and other.creator_id == nil and other.id == 2)
mv.start(function()
  local root = proc.spawn(Root, {}, {attached = false})
  assert(root.parent == nil and root.creator_id == nil)
end)
assert(seen == 3 and early.status == "dead" and other.status == "dead")
