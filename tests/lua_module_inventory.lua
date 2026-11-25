-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local function read(path)
  local file = assert(io.open(path, "rb"))
  local contents = file:read("a")
  file:close()
  return contents
end
local globals = read("docs/api/lua-globals.md")
local namespaces = read("docs/api/lua-namespaces.md")
local known = {import_globals = true, legacy_fpg = true, colors = true}
assert(globals:find("`mv.import_globals`", 1, true))
assert(globals:find("`mv.legacy_fpg`", 1, true))
assert(type(mv.import_globals) == "function" and type(mv.legacy_fpg) == "function")
for name in globals:gmatch("| `([%w_]+)` | `mv%.[%w_]+` |") do known[name] = true end
for name in namespaces:gmatch("| `mv%.([%w_]+)%.[%w_]+` |") do known[name] = true end
for name in pairs(mv) do
  assert(known[name], "undocumented module field: " .. name)
end
assert(namespaces:find("`mv.colors`", 1, true) and type(mv.colors) == "table")
for name, field in globals:gmatch("| `([%w_]+)` | `mv%.colors%.([%w_]+)` |") do
  assert(name == field and mv.colors[field], "missing palette export: " .. name)
end
local asset_option = assert(os.getenv("MALVADO_INVENTORY_ASSETS"))
assert(asset_option == "0" or asset_option == "1")
local has_assets = asset_option == "1"
local optional = {animation=true, graph=true, parse_atlas=true, parse_aseprite=true, atlas_sprite_info=true, atlas_sprite_id=true,
  atlas_animation_frame=true, atlas_animation_color=true, atlas_animation_repeats=true,
  animation_elapsed=true, animation_finished=true, atlas_pivot_color=true, draw_sprite=true, animation_sample=true,
  fpg_count=true, fpg_id_at=true, fpg_palette=true, fpg_gamma=true,
  fpg_sprite=true, fpg_point=true, fpg_rgba=true, fpg_image=true, fpg_png=true, fpg_json=true,
  manifest_count=true, manifest_id_at=true, manifest_transparent_zero=true,
  manifest_palette=true, manifest_gamma=true, manifest_sprite=true, manifest_point=true}
mv.import_globals()
for name in globals:gmatch("| `([%w_]+)` | `mv%.[%w_]+` |") do
  if has_assets or not optional[name] then
    assert(mv[name] ~= nil, "documented module field is missing: " .. name)
    assert(_G[name] == mv[name], "global does not match module: " .. name)
  else
    assert(mv[name] == nil and _G[name] == nil, "unexpected asset field: " .. name)
  end
end

assert(mv.camera == nil and mv.world == nil)
local objects = {
  world = mv.world_create(),
  camera = mv.camera_create {},
  graphic = mv.rect {width = 1, height = 1},
  shape = mv.collision.circle {radius = 1},
  region = mv.region {x = 0, y = 0, width = 1, height = 1},
  scroll = mv.scroll(1, 0),
}
if has_assets then
  objects.atlas = mv.load_asset("fixtures/atlas.json")
  local fpg = mv.fpg.parse(read("tests/fixtures/fpg/bennugd-8.fpg"))
  objects.animation = mv.animation(fpg, {7, 42})
  objects.fpg = fpg
  objects.manifest = mv.fpg.parse_manifest(mv.fpg_json(fpg))
end
local methods = read("docs/api/lua-instance-methods.md")
local expected = {}
for receiver, method in methods:gmatch("| `([%w_]+):([%w_]+)%(") do
  local names = expected[receiver] or {}
  assert(not names[method], "duplicate method in inventory: " .. receiver .. ":" .. method)
  names[method] = true
  expected[receiver] = names
end

local function check_methods(name, object, entries, names)
  assert(type(object) == "userdata" and type(getmetatable(object)) == "string", name)
  if not names then
    assert(entries == nil, "unexpected member access: " .. name)
    return
  end
  assert(type(entries) == "table", "missing method table: " .. name)
  for key in pairs(names) do
    assert(type(object[key]) == "function", "missing method: " .. name .. ":" .. key)
  end
  for key in pairs(entries) do
    assert(names[key], "undocumented method: " .. name .. ":" .. key)
  end
end

for name, object in pairs(objects) do
  check_methods(name, object, debug.getmetatable(object).__index, expected[name])
end

-- Colors expose channels as data. Process handles expose state, custom fields and methods.
for _, key in ipairs({"r", "g", "b", "a", 1, 2, 3, 4}) do
  assert(type(mv.colors.RED[key]) == "number")
end
local Probe = mv.process(function(self)
  local _, entries = debug.getupvalue(debug.getmetatable(self).__index, 1)
  if not has_assets then
    assert(self.animation_elapsed == nil)
    assert(self.animation_finished == nil)
    expected.handle.animation_elapsed = nil
    expected.handle.animation_finished = nil
  end
  check_methods("handle", self, entries, expected.handle)
  assert(self:recv() == nil and recv(self) == nil)
end)
mv.start(function() Probe() end)
