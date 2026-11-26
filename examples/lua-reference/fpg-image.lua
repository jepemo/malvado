-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")

local show = mv.process(function()
    local source <close> = assert(io.open(mv.get_assets_dir() .. "/graphics.fpg", "rb"))
    local library = mv.fpg.parse(source:read("a"))
    local image = assert(library:image(7, {transparent_zero = true}))
    local width, height = image:size()
    print(("Sprite image: %dx%d"):format(width, height))
    for _ = 1, 120 do
        mv.draw_image {image = image, x = 20, y = 20, scale = 20}
        mv.frame()
    end
end)

mv.start(function() show() end, {title = "FPG sprite image", width = 120, height = 100})
