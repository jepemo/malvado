-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local file = assert(io.open("docs/api/lua-namespaces.md", "rb"))
local documentation = file:read("a")
file:close()

local namespaces = {"process", "input", "graphics", "collision", "fpg"}
local known, documented = {colors = true}, {}
for _, namespace in ipairs(namespaces) do known[namespace] = true end
local asset_option = assert(os.getenv("MALVADO_INVENTORY_ASSETS"))
local has_assets = asset_option == "1"
assert(has_assets or asset_option == "0")
assert((mv.fpg ~= nil) == has_assets, "FPG namespace does not match the asset build")
assert((mv.graphics.graph ~= nil) == has_assets, "graph selector does not match the asset build")
assert(mv.graphics.sprite == nil, "obsolete sprite selector is exported")
for name, value in pairs(mv) do
  if type(value) == "table" then
    assert(known[name], "undocumented module namespace: " .. name)
  end
end
for namespace, name in documentation:gmatch("| `mv%.([%w_]+)%.([%w_]+)` |") do
  assert(known[namespace], "unknown namespace in inventory: " .. namespace)
  local key = namespace .. "." .. name
  assert(not documented[key], "duplicate inventory entry: " .. key)
  documented[key] = true
  local optional = namespace == "fpg" or (namespace == "graphics" and (name == "graph" or name == "animation" or name == "animation_elapsed"))
  if not optional or has_assets then
    assert(type(mv[namespace]) == "table" and type(mv[namespace][name]) == "function",
      "documented function is missing: " .. key)
  end
end
for _, namespace in ipairs(namespaces) do
  if namespace ~= "fpg" or mv.fpg then
    assert(type(mv[namespace]) == "table", "missing namespace: " .. namespace)
    for name, value in pairs(mv[namespace]) do
      if type(value) == "function" then
        assert(documented[namespace .. "." .. name], "undocumented function: " .. namespace .. "." .. name)
      end
    end
  end
end
