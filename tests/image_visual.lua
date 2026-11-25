-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
assert(not pcall(mv.load_image, "fixtures/red.png"))
assert(mv.draw_image_region == nil)
mv.import_globals()
assert(draw_image_region == nil and draw_image == mv.draw_image)
local shared
local producer = mv.process(function() shared = mv.load_image("fixtures/red.png") end)
local worker = mv.process(function(self)
    local image = shared
    assert(image and mv.image_size(image) == 8)
    local width, height = mv.image_size(image)
    assert(width == 8 and height == 8)
    local method_width, method_height = image:size()
    assert(method_width == width and method_height == height)
    assert(image.size(image) == width)
    assert(not pcall(image.size, nil) and not pcall(image.size, self))
    local function draw_count()
        for _, process in ipairs(mv.snapshot().processes) do
            if process.id == self.id then return process.draw_commands end
        end
        error("worker missing from snapshot")
    end
    local function rejects(fn, options, ...)
        local before = draw_count()
        assert(not pcall(fn, options, ...))
        assert(draw_count() == before, "invalid draw changed the command count")
    end
    rejects(mv.draw_image, image, 0, 0)
    rejects(mv.draw_image, {image = image, x = 0, y = 0}, false)
    rejects(mv.draw_image, {x = 0, y = 0})
    rejects(mv.draw_image, {image = {}, x = 0, y = 0})
    rejects(mv.draw_image, {image = image, x = 0})
    rejects(mv.draw_image, {image = image, y = 0})
    rejects(mv.draw_image, setmetatable({image = image}, {__index = {x = 0, y = 0}}))
    local function bad_image_option(key, value)
        local options = {image = image, x = 0, y = 0}
        options[key] = value
        rejects(mv.draw_image, options)
    end
    for _, value in ipairs({"0", false, math.huge, -math.huge, 0 / 0, 1e39}) do
        bad_image_option("x", value)
        bad_image_option("y", value)
        bad_image_option("angle", value)
        bad_image_option("scale", value)
    end
    for _, value in ipairs({0, -1, 1e-300, 1e38}) do
        bad_image_option("scale", value)
    end
    for _, value in ipairs({false, "middle", "center\0", {}, {x = 1},
                             {x = 0, y = 0, z = 0}, {x = 1e39, y = 0},
                             {x = 0, y = "1"}, {x = 0, y = math.huge}}) do
        bad_image_option("pivot", value)
    end
    for _, value in ipairs({false, {}, {x = -1, y = 0, width = 1, height = 1},
                             {x = 7, y = 0, width = 2, height = 1},
                             {x = 0, y = 0, width = 1, height = 0},
                             {x = 0, y = 0, width = 1, height = -1},
                             {x = 0, y = 0, width = 1.5, height = 1},
                             {x = 2^40, y = 0, width = 1, height = 1},
                             {x = "0", y = 0, width = 1, height = 1},
                             {x = 0, y = 0, width = 1, height = 1, extra = true}}) do
        bad_image_option("source", value)
    end
    bad_image_option("rotation", 0)
    bad_image_option("x\0", 0)
    bad_image_option(1, true)
    local file = assert(io.open("tests/fixtures/atlas.json", "rb"))
    local json = file:read("a")
    file:close()
    local atlas
    if mv.load_atlas then
        assert(not pcall(mv.load_atlas, image, "\11" .. json))
        atlas = mv.load_atlas(image, json)
        local animated_file = assert(io.open("tests/fixtures/animated-atlas.json", "rb"))
        local animated_json = animated_file:read("a")
        animated_file:close()
        local named_native = animated_json:gsub('"name": "red"', '"name": "niño"')
            :gsub('"name": "flash"', '"name": "camiñar"')
            :gsub('"image": "red.png"', '"image": "niño.png"')
        local utf8_atlas = mv.load_atlas(image, named_native)
        assert(mv.sprite_id(utf8_atlas, "niño") == 7)
        assert(mv.animation_frame(utf8_atlas, "camiñar", 100) == 42)
        local animated = mv.load_atlas(image, animated_json)
        assert(mv.animation_frame(animated, "flash", 99) == 7)
        assert(mv.animation_frame(animated, "flash", 100) == 42)
        assert(mv.animation_frame(animated, "flash", 300) == 7)
        assert(mv.animation_frame(animated, "flash", 300, false) == 42)
        assert(not pcall(mv.animation_frame, animated, "flash", -1))
        assert(not pcall(mv.animation_frame, animated, "missing", 0))
        assert(not pcall(mv.load_atlas, image, "{}"))
        local real_image = mv.load_image("fixtures/aseprite/coric/coric.png")
        local real_file = assert(io.open("tests/fixtures/aseprite/coric/coric.json", "rb"))
        local real_atlas = mv.load_aseprite(real_image, real_file:read("a"))
        real_file:close()
        assert(mv.sprite_id(real_atlas, "coric 1.aseprite") == 2)
        assert(mv.animation_frame(real_atlas, "walk_n", 132) == 1)
        assert(mv.animation_frame(real_atlas, "walk_n", 133) == 2)
        assert(mv.animation_frame(real_atlas, "walk_n", 267) == 3)
        assert(mv.animation_frame(real_atlas, "walk_n", 400) == 2)
        local ase_file = assert(io.open("tests/fixtures/aseprite/array.json", "rb"))
        local ase_json = ase_file:read("a")
        ase_file:close()
        assert(not pcall(mv.load_aseprite, image, "\12" .. ase_json))
        local fractional = ase_json:gsub('"duration":100', '"duration":100.0000000000000001')
        assert(not pcall(mv.load_aseprite, image, fractional))
        local bad_format, format_count = ase_json:gsub('"format":"RGBA8888"', '"format":"I8"')
        assert(format_count == 1)
        local ok, message = pcall(mv.load_aseprite, image, bad_format)
        assert(not ok and message:find("unsupported Aseprite pixel format", 1, true))
        local bad_scale, scale_count = ase_json:gsub('"scale":"1"', '"scale":"2"')
        assert(scale_count == 1)
        ok, message = pcall(mv.load_aseprite, image, bad_scale)
        assert(not ok and message:find("unsupported Aseprite scale", 1, true))
        ok, message = pcall(mv.load_aseprite, image,
            '{"frames":{},"meta":{"image":"red.png","size":{"w":8,"h":8}}}')
        assert(not ok and message:find("unsupported Aseprite JSON hash; export JSON array", 1, true))
        local multiple_slices, slice_count = ase_json:gsub('"slices":%[%]', '"slices":[{},{}]')
        assert(slice_count == 1)
        ok, message = pcall(mv.load_aseprite, image, multiple_slices)
        assert(not ok and message:find("unsupported Aseprite multiple slices", 1, true))
        local unicode_path, path_count = ase_json:gsub('"image":"red.png"', '"image":"niño.png"')
        assert(path_count == 1 and mv.load_aseprite(image, unicode_path))
        local invalid_path = ase_json:gsub('"image":"red.png"', '"image":"\\u007f.png"')
        assert(not pcall(mv.load_aseprite, image, invalid_path))
        local unicode_json, unicode_count = ase_json:gsub('"filename":"one"', '"filename":"niño"')
        assert(unicode_count == 1)
        local unicode = mv.load_aseprite(image, unicode_json)
        assert(mv.sprite_id(unicode, "niño") == 1 and mv.sprite_info(unicode, 1).name == "niño")
        local ase = mv.load_aseprite(image, ase_json)
        assert(mv.animation_frame(ase, "default", 99) == 1)
        assert(mv.animation_frame(ase, "default", 100) == 2)
        local tagged_json = ase_json:gsub('"frameTags":%[%]',
            '"frameTags":[{"name":"back","from":0,"to":1,"direction":"reverse"}]')
        local named_json, named_count = tagged_json:gsub('"name":"back"', '"name":"camiñar"')
        assert(named_count == 1)
        local named = mv.load_aseprite(image, named_json)
        assert(mv.animation_frame(named, "camiñar", 200) == 1)
        local tagged = mv.load_aseprite(image, tagged_json)
        assert(mv.animation_frame(tagged, "back", 199) == 2)
        assert(mv.animation_frame(tagged, "back", 200) == 1)
        local bounce_file = assert(io.open("tests/fixtures/aseprite/pingpong.json", "rb"))
        local bounce = mv.load_aseprite(image, bounce_file:read("a"))
        bounce_file:close()
        assert(mv.animation_repeats(bounce, "bounce") == 3)
        assert(mv.animation_repeats(bounce, "single") == 0)
        assert(mv.animation_repeats(bounce, "default") == nil)
        assert(mv.animation_repeats(bounce, "missing") == nil)
        assert(not pcall(mv.animation_repeats, bounce, "bounce\0bad"))
        local color = mv.animation_color(bounce, "bounce")
        assert(color[1] == 18 and color[2] == 171 and color[3] == 205 and color[4] == 128)
        color[1] = 0
        assert(mv.animation_color(bounce, "bounce")[1] == 18)
        assert(mv.animation_color(bounce, "default") == nil)
        assert(mv.animation_color(bounce, "missing") == nil)
        assert(not pcall(mv.animation_color, bounce, "bounce\0other"))
        assert(mv.animation_frame(bounce, "bounce", 300) == 3)
        assert(mv.animation_frame(bounce, "bounce", 600) == 2)
        assert(mv.animation_frame(bounce, "bounce", 800) == 1)
        assert(mv.animation_frame(bounce, "backbounce", 500) == 1)
        assert(not pcall(mv.load_aseprite, image, "{}"))
        mv.draw_sprite {atlas = ase, id = 2, x = -100, y = -100}
        assert(draw_sprite == mv.draw_sprite)
        rejects(mv.draw_sprite, atlas, 7, 0, 0)
        rejects(mv.draw_sprite, {id = 7, x = 0, y = 0})
        rejects(mv.draw_sprite, {atlas = atlas, x = 0, y = 0})
        rejects(mv.draw_sprite, {atlas = image, id = 7, x = 0, y = 0})
        rejects(mv.draw_sprite, {atlas = atlas, id = 7, x = 0})
        local function bad_sprite_option(key, value)
            local options = {atlas = atlas, id = 7, x = 0, y = 0}
            options[key] = value
            rejects(mv.draw_sprite, options)
        end
        for _, id in ipairs({0, -1, 1.5, 99, 2^40, "7", math.huge}) do
            bad_sprite_option("id", id)
        end
        bad_sprite_option("source", {})
        bad_sprite_option("pivot", "center")
        bad_sprite_option("scale", 0)
        bad_sprite_option("x", 0 / 0)
        bad_sprite_option("y", "0")
        bad_sprite_option("angle", math.huge)
    end
    assert(not pcall(mv.load_image, "fixtures/missing.png"))
    mv.draw_image {image = image, x = 10, y = 10}
    local green = {x = 4, y = 0, width = 4, height = 8}
    mv.draw_image {image = image, x = 25, y = 5, source = green, pivot = "top-left"}
    local odd = {x = 4, y = 0, width = 3, height = 3}
    draw_image {image = image, x = 8.5, y = 23.5, source = odd, pivot = "center", scale = 3}
    draw_image {
        image = image, x = 23.5, y = 23.5, source = odd, pivot = {x = 0.5, y = -0.5},
        scale = 3, angle = 90
    }
    if atlas then
        image = nil
        collectgarbage("collect")
        draw_sprite {atlas = atlas, id = 42, x = 50, y = 40, scale = 2, angle = 90}
    else
        draw_image {
            image = image, x = 50, y = 40, source = green, pivot = {x = 2, y = 4},
            scale = 2, angle = 90
        }
    end
    assert(self.graph == nil)
    mv.set_color(mv.colors.BLUE)
    mv.draw_rect {x = 2, y = 30, width = 8, height = 6}
    mv.draw_rect {x = 15, y = 45, width = 10, height = 10, filled = false, color = {255, 255, 0, 128}}
    mv.draw_rect {x = 12, y = 45, width = 1, height = 1, filled = false}
    mv.draw_circle {x = 35, y = 50, radius = 4, color = {r = 0, g = 255, b = 255}}
    mv.draw_circle {x = 55, y = 15, radius = 4, filled = false, color = mv.colors.MAGENTA}
    atlas = nil
    shared = nil
    image = nil
    collectgarbage("collect")
    mv.frame()
end)
mv.start(function()
    local image = mv.load_image("fixtures/red.png")
    local ok, message = pcall(mv.draw_image, {image = image, x = 0, y = 0})
    assert(not ok and message:find("inside a process", 1, true))
    producer()
    worker()
end, {width = 64, height = 64, title = "Lua image test"})
