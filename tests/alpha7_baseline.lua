-- Malvado 2; created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local drops = 0
local function guard()
  return setmetatable({}, {__close = function() drops = drops + 1 end})
end
local function record(name, value)
  print(string.format("%d %s %d", mv.snapshot().tick, name, value))
end
local Child = mv.process(function()
  local cleanup <close> = guard()
  local value = 7
  record("child", value)
  mv.frame()
  value = value + 1
  record("child", value)
end)
local Parent = mv.process(function()
  local cleanup <close> = guard()
  local value = 40
  record("parent", value)
  mv.process.spawn(Child, {}, {attached = false})
  mv.frame()
  value = value + 1
  record("parent", value)
end)
local Slow = mv.process(function()
  local cleanup <close> = guard()
  for value = 1, 3 do
    record("slow", value)
    if value < 3 then mv.frame() end
  end
end)
mv.start(function()
  Parent()
  Slow {fps = 15}
end)
assert(drops == 3)
print(string.format("closed %d", drops))
