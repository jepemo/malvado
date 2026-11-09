-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0
require("malvado").import_globals()
local atlas, definition
local metadata = load_asset("examples/aseprite/sheet.json")
local metadata_definition = animation(metadata, "bounce")
local Actor = process(function(self)
  while true do
    self.graph = animation("bounce") -- Repeated lookup must not restart playback.
    frame()
  end
end)
local Controller = process(function()
  local tick = 1
  Actor({file = atlas, graph = definition, x = 30, y = 20, size = 4, fps = 5})
  while true do
    if tick == 7 then Actor({file = atlas, graph = definition, x = 70, y = 20, size = 4}) end
    tick = tick + 1
    frame()
  end
end)
return {width = 120, height = 80, init = function()
  clear_screen(0, 0, 0)
  local ok, message = pcall(animation, metadata, "bounce")
  assert(not ok and message:find("no visual images", 1, true))
  assert(metadata_definition)
  atlas = load_asset("examples/aseprite/sheet.json")
  definition = animation(atlas, "bounce")
  Controller()
end}
