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
local known = {import_globals = true, legacy_fpg = true}
assert(globals:find("`mv.import_globals`", 1, true))
assert(globals:find("`mv.legacy_fpg`", 1, true))
assert(type(mv.import_globals) == "function" and type(mv.legacy_fpg) == "function")
for name in globals:gmatch("| `([%w_]+)` | `mv%.[%w_]+` |") do known[name] = true end
for name in namespaces:gmatch("| `mv%.([%w_]+)%.[%w_]+` |") do known[name] = true end
for name in pairs(mv) do
  assert(known[name], "undocumented module field: " .. name)
end
local asset_option = assert(os.getenv("MALVADO_INVENTORY_ASSETS"))
assert(asset_option == "0" or asset_option == "1")
local has_assets = asset_option == "1"
local optional = {load_atlas=true, load_aseprite=true, sprite_info=true, sprite_id=true,
  animation_frame=true, animation_color=true, atlas_pivot_color=true,
  animation_repeats=true, draw_sprite=true}
for name in globals:gmatch("| `([%w_]+)` | `mv%.[%w_]+` |") do
  if has_assets or not optional[name] then
    assert(mv[name] ~= nil, "documented module field is missing: " .. name)
  else
    assert(mv[name] == nil, "unexpected asset field: " .. name)
  end
end

local objects = {world = mv.world.create()}
if has_assets then
  local fpg = mv.fpg.parse(read("tests/fixtures/fpg/bennugd-8.fpg"))
  objects.fpg = fpg
  objects.manifest = mv.fpg.parse_manifest(fpg:json())
end
local documented = {}
for object, method in namespaces:gmatch("`([%w_]+):([%w_]+)`") do
  documented[object] = documented[object] or {}
  assert(not documented[object][method], "duplicate method: " .. object .. ":" .. method)
  documented[object][method] = true
end
for name, object in pairs(objects) do
  local methods = debug.getmetatable(object).__index
  for method, value in pairs(methods) do
    assert(type(value) == "function" and documented[name] and documented[name][method],
      "undocumented method: " .. name .. ":" .. method)
  end
  for method in pairs(assert(documented[name])) do
    assert(type(methods[method]) == "function", "missing method: " .. name .. ":" .. method)
  end
end
