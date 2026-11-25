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
fails("running process", graph, "green")
fails("expected graph", graph)
fails("requires an atlas", graph, {}, 42)
local atlas = load_asset("fixtures/atlas.json")
local aseprite = load_asset("../examples/aseprite/sheet.json")
local fpg = load_asset("../examples/fpg/graphics.fpg")
local explicit = graph(atlas, 42)
local namespaced = mv.graphics.graph(atlas, "green")
local module = mv.graph(atlas, "green")
assert(explicit == namespaced and explicit == module)
assert(atlas:graph("green") == explicit)
local trimmed = aseprite:graph("one")
assert(trimmed == graph(aseprite, 1))
assert(mv.graph(fpg, 7) == graph(fpg, 7))
assert(not pcall(function() explicit.pivot_x = 0 end))
fails("unknown", graph, atlas, 999)
fails("unknown sprite name", graph, atlas, "Green")
fails("unknown sprite name", graph, atlas, "42")
fails("invalid sprite name", graph, atlas, "green\0hidden")
fails("unknown", graph, fpg, 999)
fails("names require an atlas", graph, fpg, "7")
fails("unknown", graph, atlas, -1)
fails("integer", graph, atlas, 1.5)
fails("number expected", graph, atlas, {})
local file = assert(io.open("examples/fpg/graphics.fpg", "rb"))
local parsed = mv.fpg.parse(file:read("a"))
file:close()
assert(graph(parsed, 7))
local manifest = mv.fpg.parse_manifest(fpg_json(fpg, {transparent_zero = true}))
assert(mv.graph(manifest, 7) == graph(manifest, 7)) -- Headless geometry does not need PNGs.
fails("names require an atlas", manifest.graph, manifest, "7")
local fallback_json, replacements = fpg_json(fpg, {transparent_zero = true}):gsub(
  '{"x":1,"y":0,"defined":true}', '{"x":-1,"y":-1,"defined":false}', 1)
assert(replacements == 1)
local fallback = mv.fpg.parse_manifest(fallback_json)
local world = world_create()
world_add_rect(world, {x = 8, y = 6, width = 1, height = 1})
local Defaulted = process(function(self)
  assert(self.file == atlas and self.world == world)
  self.graph = graph(42)
  assert(self.graph == explicit and world_collision() == 1)
  local x, y = self.x, self.y
  assert(world_collision {x = x + 100} == nil)
  assert(self.x == x and self.y == y)
  self.graph = nil
end, {defaults = {file = atlas, world = world}})
local Actor = process(function() while true do frame() end end)
local Player = process(function(self)
  fails("no file", graph, "green")
  self.file = false
  fails("requires an atlas", graph, 42)
  self.file = atlas
  fails("no visual image", draw_sprite, {atlas = atlas, id = 42, x = 0, y = 0})
  self.graph = graph("green")
  assert(self.graph == explicit and world_collision(world, self) == 1)
  self.file = fpg
  fails("names require an atlas", graph, "42")
  assert(world_collision(world, self) == 1) -- Selection still belongs to atlas.
  self.file = nil
  assert(world_collision(world, self) == 1)
  for _, descriptor in ipairs({explicit, namespaced, module}) do
    self.graph = descriptor
    assert(world_collision(world, self) == 1)
  end
  self.graph = graph(fpg, 42)
  assert(world_collision(world, self) == nil)
  self.file = fpg
  self.graph = graph(42)
  assert(world_collision(world, self) == nil)
  self.graph = graph(fpg, 7)
  local point_world = world_create()
  world_add_rect(point_world, {x = 9, y = 10, width = 1, height = 1})
  assert(world_collision(point_world, self) == 1) -- Control point 0 is (1,0).
  self.graph = graph(manifest, 7)
  assert(world_collision(point_world, self) == 1)
  self.graph = graph(fallback, 7)
  assert(world_collision(point_world, self) == nil) -- Undefined point 0 uses top-left.
  self.file = aseprite
  self.graph = graph("one")
  assert(self.graph == trimmed and world_collision(point_world, self) == 1)
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
  local descriptor = graph(atlas, 7)
  local cached = setmetatable({descriptor}, {__mode = "v"})
  descriptor = nil
  collectgarbage("collect")
  assert(cached[1] == nil) -- A live library does not pin unused descriptors.
  local transient = load_asset("fixtures/atlas.json")
  self.graph = transient:graph(42)
  local weak = setmetatable({transient, self.graph}, {__mode = "v"})
  transient = nil
  self.file = fpg
  collectgarbage("collect")
  assert(weak[1] and world_collision(world, self) == 1)
  self.graph = nil
  collectgarbage("collect")
  collectgarbage("collect")
  assert(weak[1] == nil and weak[2] == nil)
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
