-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")

local source <close> = assert(io.open(mv.get_assets_dir() .. "/graphics.fpg", "rb"))
local library = mv.fpg.parse(source:read("a"))
local options = {transparent_zero = true}
local manifest = mv.fpg.parse_manifest(library:json(options))

assert(library:count() == 2 and manifest:count() == 2)
assert(library:palette() == manifest:palette())
assert(library:gamma() == manifest:gamma())
assert(manifest:transparent_zero())

for index = 1, library:count() do
    local id = library:id_at(index)
    local sprite = library:sprite(id)
    local entry = manifest:sprite(id)
    assert(manifest:id_at(index) == id)
    local point = library:point(id, 0)
    local saved_point = manifest:point(id, 0)
    assert(point.x == saved_point.x and point.y == saved_point.y)
    assert(#library:rgba(id, options) == sprite.width * sprite.height * 4)
    assert(library:png(id, options):sub(1, 8) == "\137PNG\r\n\26\n")
    print(("Sprite %d: %dx%d, %s"):format(id, entry.width, entry.height, entry.image))
end
