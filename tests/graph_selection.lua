-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
mv.import_globals()
assert(graph == mv.graph and graph == mv.graphics.graph)
local function fails(fragment, fn, ...)
  local ok, message = pcall(fn, ...)
  assert(not ok and tostring(message):find(fragment, 1, true), tostring(message))
end
fails("running process", graph, 42)
fails("expected graph", graph)
fails("requires an atlas", graph, {}, 42)
local atlas = load_asset("tests/fixtures/atlas.json")
local fpg = load_asset("examples/fpg/graphics.fpg")
local explicit = graph(atlas, 42)
local method = atlas:graph(42)
local native_namespace = mv.graphics.graph(atlas, 42)
assert(not pcall(function() explicit.pivot_x = 0 end))
fails("unknown", graph, atlas, 999)
fails("unknown", graph, fpg, 999)
fails("unknown", graph, atlas, -1)
fails("integer", graph, atlas, 1.5)
local file = assert(io.open("examples/fpg/graphics.fpg", "rb"))
local parsed = mv.fpg.parse(file:read("a"))
file:close()
assert(parsed:graph(7))
local manifest = mv.fpg.parse_manifest(fpg:json(true))
assert(manifest:graph(7)) -- Headless geometry does not need PNGs.
local fallback_json, replacements = fpg:json(true):gsub(
  '{"x":1,"y":0,"defined":true}', '{"x":-1,"y":-1,"defined":false}', 1)
assert(replacements == 1)
local fallback = mv.fpg.parse_manifest(fallback_json)
local world = world_create()
world:add_rect({x = 8, y = 6, width = 1, height = 1})
local Defaulted = process(function(self)
  assert(self.file == atlas and self.world == world)
  self.graph = graph(42)
  assert(world:collision(self) == 1)
  self.graph = nil
end, {defaults = {file = atlas, world = world}})
local Actor = process(function() while true do frame() end end)
local Player = process(function(self)
  fails("no file", graph, 42)
  self.file = false
  fails("requires an atlas", graph, 42)
  self.file = atlas
  self.graph = graph(42)
  assert(world:collision(self) == 1)
  self.file = fpg
  assert(world:collision(self) == 1) -- Selection still belongs to atlas.
  self.file = nil
  assert(world:collision(self) == 1)
  for _, descriptor in ipairs({explicit, method, native_namespace}) do
    self.graph = descriptor
    assert(world:collision(self) == 1)
  end
  self.graph = graph(fpg, 42)
  assert(world:collision(self) == nil)
  self.file = fpg
  self.graph = graph(42)
  assert(world:collision(self) == nil)
  self.graph = fpg:graph(7)
  local point_world = world_create()
  point_world:add_rect({x = 9, y = 10, width = 1, height = 1})
  assert(point_world:collision(self) == 1) -- Control point 0 is (1,0).
  self.graph = manifest:graph(7)
  assert(point_world:collision(self) == 1)
  self.graph = fallback:graph(7)
  assert(point_world:collision(self) == nil) -- Undefined point 0 uses top-left.
  local shared = graph(atlas, 42)
  local peer = Actor({x = 10, y = 10, file = fpg, graph = shared})
  self.graph = shared
  assert(process.collision(self, peer) == peer)
  peer.file = nil
  assert(process.collision(self, peer) == peer)
  kill(peer)
  local nested = coroutine.create(function()
    fails("running process", graph, 42)
    assert(graph(atlas, 42))
  end)
  local ok, message = coroutine.resume(nested)
  assert(ok, message)
  local transient = load_asset("tests/fixtures/atlas.json")
  local weak = setmetatable({transient}, {__mode = "v"})
  self.graph = graph(transient, 42)
  transient = nil
  self.file = fpg
  collectgarbage("collect")
  assert(weak[1] and world:collision(self) == 1)
  self.graph = nil
  collectgarbage("collect")
  collectgarbage("collect")
  assert(weak[1] == nil)
end)
local game = {}
function game.init()
  Defaulted({x = 10, y = 10})
  Defaulted({x = 10, y = 10})
  Player({x = 10, y = 10})
  -- The explicit form works in initialization without a running process.
  assert(graph(atlas, 42))
end
return game
