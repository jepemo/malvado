-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
assert(not pcall(mv.load_image, "tests/fixtures/red.png"))
local worker = mv.process(function()
    local image = mv.load_image("tests/fixtures/red.png")
    local width, height = mv.image_size(image)
    assert(width == 8 and height == 8)
    assert(not pcall(mv.load_image, "tests/fixtures/missing.png"))
    mv.draw_image(image, 10, 10)
    image = nil
    collectgarbage("collect")
    mv.frame()
end)
mv.start(function()
    local image = mv.load_image("tests/fixtures/red.png")
    assert(not pcall(mv.draw_image, image, 0, 0))
    worker()
end, {width = 64, height = 64, title = "Lua image test"})
