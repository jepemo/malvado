-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local standard_load = load
assert(rect == nil and circle == nil and region == nil and scroll == nil)
-- The namespace and minimal API use identical constructor closures.
for _, name in ipairs({"rect", "circle", "region", "scroll"}) do
  assert(mv[name] == mv.graphics[name])
end
mv.import_globals()
mv.import_globals()
assert(load == standard_load and assert(load("return 42"))() == 42)
for _, name in ipairs({"rect", "circle", "region", "scroll"}) do
  assert(_G[name] == mv.graphics[name])
end
assert(graphics == nil and shape == nil and collision == mv.collision)
local function invalid(fn, ...)
  assert(not pcall(fn, ...))
end
invalid(rect, 0, 2)
invalid(rect, 1, math.huge)
invalid(rect, 1, 2, {fill = false})
invalid(rect, 1, 2, {pivot = "unknown"})
invalid(rect, 1, 2, {color = {1, 2, 3, 4, 5}})
invalid(rect, 1, 2, {color = {1, 2, 256}})
invalid(circle, -1)
invalid(circle, 1e-310)
invalid(circle, 1, {pivot = "top-left"})
invalid(circle, 1, {color = {1, 2}})
invalid(circle, 1, {other = 1})
invalid(region, 0, 0, 0, 2)
invalid(region, 0, 0, 1.5, 2)
invalid(region, math.huge, 0, 2, 2)
invalid(scroll, math.huge, 0)
invalid(scroll, 0, 0/0)
local centered = rect(4, 4)
local top_left = rect(4, 4, {pivot = "top-left", color = {255, 165, 70}})
local ball = circle(1, {color = {80, 190, 255, 128}})
local clip = region(100, 100, 1, 1)
local offset = scroll(100, -20)
for _, descriptor in ipairs({centered, top_left, ball, clip, offset}) do
  invalid(function() descriptor.x = 2 end)
end
local Actor = process(function() while true do frame() end end)
local weak = setmetatable({}, {__mode = "v"})
local Controller = process(function(self)
  local box = Actor({x = 0, y = 0, graph = centered, region = clip, scroll = offset})
  local other = Actor({x = -1, y = 1, graph = ball})
  -- Centered versus top-left geometry remains identical to the namespace API.
  assert(process.collision(box, other) == other)
  box.graph = top_left
  assert(process.collision(box, other) == nil) -- Touching edge only.
  other.x = 0
  assert(process.collision(box, other) == other)
  box.graph = mv.graphics.rect(4, 4, {pivot = "top-left"})
  assert(process.collision(box, other) == other)
  assert(box.region == clip and box.scroll == offset)
  assert(box.x == 0 and box.y == 0) -- Visual descriptors do not move physics.
  kill(box)
  kill(other)
  local transient = circle(3)
  weak[1] = transient
  local once = process(function(instance) assert(instance.graph == weak[1]) end)
  once({graph = transient})
  transient = nil
  collectgarbage("collect")
  assert(weak[1])
  frame()
  frame()
  collectgarbage("collect")
  collectgarbage("collect")
  assert(weak[1] == nil)
end)
start(function() Controller() end)
