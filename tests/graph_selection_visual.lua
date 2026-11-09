-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
mv.import_globals()
local root = assert(os.getenv("MALVADO_ASSET_TEST_DIR"))
-- A metadata-only atlas cannot silently acquire images after a window opens.
local metadata = load_asset("tests/fixtures/atlas.json")
local idle = function() while true do frame() end end
local Actor = process(idle)
local producer = process(function()
  local ok, message = pcall(graph, metadata, 42)
  assert(not ok and message:find("no visual images", 1, true))
  local image = load_asset("tests/fixtures/red.png")
  assert(not pcall(graph, image, 1)) -- A standalone image needs no library lookup.
  local atlas = load_asset("tests/fixtures/atlas.json")
  local aseprite = load_asset("examples/aseprite/sheet.json")
  local fpg = load_asset("examples/fpg/graphics.fpg")
  local opaque = load_asset("examples/fpg/graphics.fpg", {transparent_zero = false})
  local manifest = load_asset(root .. "/assets/manifest.json")
  local weak = setmetatable({atlas, aseprite, fpg, manifest}, {__mode = "v"})
  local Selected = process(function(self)
    self.graph = graph(42)
    self.file = fpg -- Must keep atlas 42, not reinterpret it as FPG 42.
    while true do
      collectgarbage("collect")
      assert(weak[1] and weak[2] and weak[3] and weak[4])
      frame()
    end
  end)
  Actor({graph = image, x = 10, y = 10}, {attached = false})
  Selected({file = atlas, x = 40, y = 20}, {attached = false})
  local shared = aseprite:graph(1)
  Actor({graph = shared, x = 60, y = 20}, {attached = false})
  Actor({graph = mv.graphics.graph(fpg, 7), x = 80, y = 20}, {attached = false})
  Actor({graph = graph(manifest, 7), x = 100, y = 20}, {attached = false})
  Actor({graph = opaque:graph(7), x = 120, y = 20}, {attached = false})
  Actor({graph = shared, x = 60, y = 50}, {attached = false})
  -- Producer completion releases its native ownership. Child descriptors keep
  -- the resources alive, including when no child has the original self.file.
end)
return {width = 160, height = 120, init = function()
  clear_screen(25, 30, 45)
  producer()
end}
