-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
require("malvado.compat.legacy").import_globals()

local show = process(function(self)
    self.fpg = mv.legacy_fpg("tile.png", "image", 1, 1)
    self.fpgIndex = 0
    self.x, self.y = 20, 24
    self.size = 4

    for _ = 1, 120 do
        frame() -- The legacy frame draws self.fpg before yielding.
    end
end)

mv.start(function() show() end, {title = "Legacy image sheet", width = 160, height = 120})
