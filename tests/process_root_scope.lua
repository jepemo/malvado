-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local proc = mv.process
local seen = 0
local Root = proc.define(function(self)
  assert(self.parent == nil and self.creator_id == nil)
  seen = seen + 1
end)
local early = Root {}
assert(early.parent == nil and early.creator_id == nil)
local ok, message = pcall(proc.spawn, Root, {}, {attached = true})
assert(not ok and message:find("without an owner", 1, true))
local other = proc.spawn(Root, {}, {attached = false})
assert(other.parent == nil and other.creator_id == nil)
mv.start(function()
  local root = proc.spawn(Root, {}, {attached = false})
  assert(root.parent == nil and root.creator_id == nil)
end)
assert(seen == 3 and early.status == "dead" and other.status == "dead")
