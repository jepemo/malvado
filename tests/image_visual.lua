-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
assert(not pcall(mv.load_image, "tests/fixtures/red.png"))
local worker = mv.process(function()
    local image = mv.load_image("tests/fixtures/red.png")
    local width, height = mv.image_size(image)
    assert(width == 8 and height == 8)
    local file = assert(io.open("tests/fixtures/atlas.json", "rb"))
    local json = file:read("a")
    file:close()
    local atlas
    if mv.load_atlas then
        atlas = mv.load_atlas(image, json)
        assert(not pcall(mv.load_atlas, image, "{}"))
        assert(not pcall(mv.draw_sprite, atlas, 99, 0, 0))
    end
    assert(not pcall(mv.load_image, "tests/fixtures/missing.png"))
    assert(not pcall(mv.draw_image_region, image, 0, 0, 7, 0, 2, 1))
    assert(not pcall(mv.draw_image_region, image, 0, 0, 0, 0, 1, 1, 0, 0, 0))
    assert(not pcall(mv.draw_image_region, image, math.huge, 0, 0, 0, 1, 1))
    assert(not pcall(mv.draw_image_region, image, 0, 0, 2^40, 0, 1, 1))
    assert(not pcall(mv.draw_image_region, image, 0 / 0, 0, 0, 0, 1, 1))
    mv.draw_image(image, 10, 10)
    mv.draw_image_region(image, 25, 5, 4, 0, 4, 8)
    if atlas then
        image = nil
        collectgarbage("collect")
        mv.draw_sprite(atlas, 42, 50, 40, 2, 90)
    else
        mv.draw_image_region(image, 50, 40, 4, 0, 4, 8, 2, 4, 2, 90)
    end
    atlas = nil
    image = nil
    collectgarbage("collect")
    mv.frame()
end)
mv.start(function()
    local image = mv.load_image("tests/fixtures/red.png")
    assert(not pcall(mv.draw_image, image, 0, 0))
    assert(not pcall(mv.draw_image_region, image, 0, 0, 0, 0, 4, 4))
    worker()
end, {width = 64, height = 64, title = "Lua image test"})
