-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
mv.import_globals()
local root = assert(os.getenv("MALVADO_ASSET_TEST_DIR"))
local function write(name, bytes)
  local file = assert(io.open(root .. "/" .. name, "wb"))
  assert(file:write(bytes))
  assert(file:close())
end
local function fails(fragment, ...)
  local ok, message = pcall(load_asset, ...)
  assert(not ok and tostring(message):find(fragment, 1, true), tostring(message))
end
return {width = 160, height = 120, init = function()
  clear_screen {25, 30, 45, 255}
  local image = load_asset("fixtures/red.png")
  local atlas = load_asset("fixtures/atlas.json")
  local aseprite = load_asset("../examples/aseprite/sheet.json")
  local fpg = load_asset("../examples/fpg/graphics.fpg")
  set_assets_dir(root .. "/first")
  local first = load_image("palette.png")
  assert(image_size(first) == image_size(load_asset(root .. "/first/palette.png")))
  set_assets_dir(root .. "/second")
  assert(image_size(first) ~= image_size(load_asset("palette.png")))
  set_assets_dir(root)
  local manifest = load_asset("assets/manifest.json")
  assert(manifest_palette(manifest) == fpg_palette(fpg) and manifest_gamma(manifest) == fpg_gamma(fpg))
  local converted = load_asset("assets/7.png")
  assert(mv.atlas_sprite_info(atlas, 42).pivot_x == 2)
  assert(mv.atlas_sprite_info(aseprite, 1).trim_x == 2)
  assert(manifest_point(manifest, 7, 0).x == fpg_point(fpg, 7, 0).x)
  write("missing.json", '{"schema":1,"kind":"graphics-library","image":"absent.png","sprites":[{"id":7,"name":"test","x":0,"y":0,"width":1,"height":1}]}')
  fails("could not load PNG", root .. "/missing.json")
  local file = assert(io.open("tests/fixtures/atlas.json", "rb"))
  local json = file:read("a")
  file:close()
  -- Resolve an absolute image path, then reject sprite bounds outside the PNG.
  local cwd = assert(os.getenv("MALVADO_ASSET_SOURCE"))
  local bad = json:gsub('"red.png"', '"' .. cwd .. '/tests/fixtures/red.png"'):gsub('"width": 4', '"width": 400')
  write("bounds.json", bad)
  fails("invalid atlas", root .. "/bounds.json")
  local manifest_file = assert(io.open(root .. "/assets/manifest.json", "rb"))
  local metadata = manifest_file:read("a")
  manifest_file:close()
  write("assets/bad.json", metadata:gsub('"width":2', '"width":3'))
  fails("dimensions do not match", root .. "/assets/bad.json")
  local weak = setmetatable({atlas, aseprite, fpg, manifest}, {__mode = "v"})
  local Picture = process(function()
    while true do
      draw_image {image = image, x = 10, y = 10}
      draw_sprite {atlas = atlas, id = 42, x = 40, y = 20}
      draw_sprite {atlas = aseprite, id = 1, x = 60, y = 20}
      draw_image {image = fpg_image(fpg, 7, {transparent_zero = true}), x = 80, y = 20}
      draw_image {image = converted, x = 100, y = 20}
      collectgarbage("collect")
      assert(weak[1] and weak[2] and weak[3])
      frame()
    end
  end)
  Picture()
end}
