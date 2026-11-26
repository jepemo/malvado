-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")

local actor = mv.process(function()
    while not mv.key(mv.KEY_ESCAPE) do mv.frame() end
end)

return {
    title = "Sharing sprites", width = 320, height = 160, assets_dir = "assets",
    init = function()
        local sheet = mv.load_asset("sheet.json")
        local sprite = mv.graph(sheet, "one")
        actor {x = 88, y = 80, size = 12, graph = sprite}
        actor {x = 232, y = 80, size = 12, angle = 45, graph = sprite}
    end,
}
