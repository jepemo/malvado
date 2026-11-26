-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")

local show = mv.process(function()
    local builtin = mv.font {size = 20}
    local custom = mv.load_font {path = "AnonymousPro-Bold.ttf", size = 20}
    local label = mv.write {font = builtin, text = "Score: 0", x = 160, y = 40,
                           align = mv.TEXT_ALIGN_CENTER}
    for _ = 1, 60 do mv.frame() end

    mv.change_text(label, {font = custom, text = "Score: 10", color = mv.colors.YELLOW})
    local width, height = mv.text_size(label)
    assert(width > 0 and height > 0)
    print(("Score label: %d × %d pixels"):format(width, height))
    for _ = 1, 60 do mv.frame() end

    mv.delete_text(label)
    for _ = 1, 30 do mv.frame() end
end)

mv.start(function() show() end, {title = "Persistent text", width = 320, height = 120})
