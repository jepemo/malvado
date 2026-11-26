-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")

local show = mv.process(function(self)
    local image = mv.load_image("sheet.png")
    local atlas = mv.parse_atlas(image, [[
      {"schema":1,"kind":"graphics-library","image":"sheet.png",
       "sprites":[{"id":7,"name":"icon","x":0,"y":0,"width":4,"height":4}]}
    ]])
    local source <close> = assert(io.open(mv.get_assets_dir() .. "/sheet.json", "rb"))
    local aseprite = mv.parse_aseprite(image, source:read("a"))
    local first = assert(aseprite:sprite_info("one"))
    assert(first.id == aseprite:sprite_id("one"))
    local color = assert(aseprite:animation_color("bounce"))
    assert(color[4] == 128)
    assert(aseprite:animation_repeats("bounce") == nil)
    assert(aseprite:pivot_color() == nil)
    print(("First sprite: %s, %dx%d"):format(first.name, first.width, first.height))

    self.file = mv.load_asset("sheet.json")
    self.graph = mv.graph("one")
    mv.frame()
    local sequence = mv.animation("bounce", {loop = false})
    assert(sequence:sample(0) == self.graph)
    self.graph = sequence

    while not self:animation_finished() do
        local elapsed = assert(self:animation_elapsed())
        local id = aseprite:animation_frame("bounce", elapsed, {loop = false})
        mv.draw_sprite {atlas = aseprite, id = id, x = 128, y = 54, scale = 8}
        mv.draw_sprite {atlas = atlas, id = 7, x = 168, y = 38, scale = 8}
        mv.frame()
    end
    print("Animation finished")
end)

mv.start(function() show {x = 48, y = 54, size = 8} end,
         {title = "Atlas frames", width = 220, height = 108})
