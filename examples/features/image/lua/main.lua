-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")

local show = mv.process(function(self)
    for _ = 1, 120 do
        mv.draw_image(self.image, 32, 32)
        mv.frame()
    end
end)

return {
    title = "Malvado Image",
    width = 160,
    height = 120,
    init = function()
        mv.clear_screen {0, 0, 0, 255}
        local image = mv.load_image("tile.png")
        local width, height = mv.image_size(image)
        print(("image: %dx%d"):format(width, height))
        show({image = image}) -- The process keeps the image for its draw calls.
    end,
}
