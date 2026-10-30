-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local process = mv.process(function()
    for _ = 1, 60 do mv.frame() end
    local text = ""
    for _ = 1, 600 do
        local character = mv.text_input_next()
        while character do
            text = text .. character
            character = mv.text_input_next()
        end
        if #text >= 3 then
            assert(text == "aZ9")
            return
        end
        mv.frame()
    end
    error("text input test timed out")
end)
mv.start(function() process() end, {width = 160, height = 120, title = "Malvado Lua text input"})
