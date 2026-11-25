-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
mv.import_globals()
local atlas = mv.load_asset("fixtures/animated-atlas.json")
local file = assert(io.open("tests/fixtures/fpg/bennugd-8.fpg", "rb"))
local fpg = mv.fpg.parse(file:read("a"))
file:close()
local manifest = mv.fpg.parse_manifest(fpg:json(true))

local function same(actual, expected)
  if type(expected) == "table" then
    assert(type(actual) == "table")
    for key, value in pairs(expected) do assert(actual[key] == value, key) end
    for key, value in pairs(actual) do assert(expected[key] == value, key) end
  else
    assert(actual == expected)
  end
end

local function check(receiver, method, procedure, colon_result, ...)
  assert(receiver[method] == procedure)
  local expected = procedure(receiver, ...)
  same(colon_result, expected)
  same(receiver[method](receiver, ...), expected)
  assert(not pcall(receiver[method], nil, ...))
end

local function check_graph(library, id)
  assert(library:graph(id) == mv.graph(library, id))
  assert(library.graph(library, id) == mv.graphics.graph(library, id))
  assert(not pcall(library.graph, {}, id))
  assert(not pcall(library.graph, library, -1))
  assert(not pcall(library.graph, id))
end

assert(atlas_animation_frame == mv.atlas_animation_frame)
assert(atlas_animation_color == mv.atlas_animation_color)
assert(atlas_animation_repeats == mv.atlas_animation_repeats)
for _, name in ipairs({"animation_frame", "animation_color", "animation_repeats"}) do
  assert(mv[name] == nil and _G[name] == nil)
end
local sprite_id = mv.atlas_animation_frame(atlas, "flash", 0)
local sprite_name = mv.sprite_info(atlas, sprite_id).name
check(atlas, "sprite_info", mv.sprite_info, atlas:sprite_info(sprite_id), sprite_id)
check(atlas, "sprite_id", mv.sprite_id, atlas:sprite_id(sprite_name), sprite_name)
check(atlas, "animation_frame", mv.atlas_animation_frame, atlas:animation_frame("flash", 300.5, {loop = false}),
  "flash", 300.5, {loop = false})
check(atlas, "animation_color", mv.atlas_animation_color, atlas:animation_color("flash"), "flash")
check(atlas, "animation_repeats", mv.atlas_animation_repeats, atlas:animation_repeats("flash"), "flash")
check(atlas, "pivot_color", mv.atlas_pivot_color, atlas:pivot_color())
assert(atlas:sprite_id("missing") == nil and not pcall(atlas.sprite_info, atlas, -1))
assert(not pcall(atlas.animation_frame, atlas, "flash", -1))
check_graph(atlas, sprite_id)

check(fpg, "count", mv.fpg_count, fpg:count())
check(fpg, "id_at", mv.fpg_id_at, fpg:id_at(1), 1)
check(fpg, "palette", mv.fpg_palette, fpg:palette())
check(fpg, "gamma", mv.fpg_gamma, fpg:gamma())
check(fpg, "sprite", mv.fpg_sprite, fpg:sprite(7), 7)
check(fpg, "point", mv.fpg_point, fpg:point(7, 0), 7, 0)
check(fpg, "rgba", mv.fpg_rgba, fpg:rgba(7, true), 7, true)
check(fpg, "png", mv.fpg_png, fpg:png(7, true), 7, true)
check(fpg, "json", mv.fpg_json, fpg:json(true), true)
check(fpg, "image", mv.fpg_image, fpg:image(999), 999)
assert(not pcall(fpg.image, fpg, 7, true)) -- Loading an existing sprite needs a window.
assert(not pcall(fpg.rgba, fpg, 7, 1))
assert(fpg:id_at(0) == nil and fpg:sprite(999) == nil and fpg:point(7, -1) == nil)
check_graph(fpg, 7)

check(manifest, "count", mv.manifest_count, manifest:count())
check(manifest, "id_at", mv.manifest_id_at, manifest:id_at(1), 1)
check(manifest, "palette", mv.manifest_palette, manifest:palette())
check(manifest, "gamma", mv.manifest_gamma, manifest:gamma())
check(manifest, "sprite", mv.manifest_sprite, manifest:sprite(7), 7)
check(manifest, "point", mv.manifest_point, manifest:point(7, 0), 7, 0)
check(manifest, "transparent_zero", mv.manifest_transparent_zero, manifest:transparent_zero())
assert(manifest:id_at(0) == nil and manifest:sprite(999) == nil)
assert(not pcall(manifest.point, fpg, 7, 0))
check_graph(manifest, 7)

local weak = setmetatable({atlas, fpg, manifest}, {__mode = "v"})
local count = fpg.count
atlas, fpg, manifest = nil, nil, nil
collectgarbage("collect")
assert(weak[1] == nil and weak[2] == nil and weak[3] == nil)
assert(count == mv.fpg_count)
