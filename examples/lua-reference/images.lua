-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")

local show = mv.process(function()
    mv.set_assets_dir("assets")
    local image = mv.load_image("tile.png")
    local width, height = image:size()
    assert(width == 32 and height == 32)
    print(("Loaded tile: %dx%d"):format(width, height))
    print("Asset directory:", mv.get_assets_dir())
    mv.set_assets_dir()

    for _ = 1, 120 do
        mv.draw_image {image = image, x = 20, y = 24}
        mv.draw_image {image = image, x = 110, y = 40, pivot = "center", angle = 45,
                       source = {x = 0, y = 0, width = 16, height = 16}, scale = 2}
        mv.frame()
    end
end)

mv.start(function() show() end, {title = "Image regions and pivots", width = 160, height = 90})
