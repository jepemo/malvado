-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local ok, message = pcall(mv.load_image, "fixtures/red.png")
assert(not ok and message:match("open window"))
assert(not pcall(mv.load_image, "a\0b.png"))
assert(not pcall(mv.image_size, {}))
local drawn, draw_error = pcall(mv.draw_image, {image = {}, x = 0, y = 0})
assert(not drawn and draw_error:find("MvImage"))
assert(mv.draw_image_region == nil)
