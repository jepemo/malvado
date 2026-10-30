-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local ok, message = pcall(mv.load_image, "tests/fixtures/red.png")
assert(not ok and message:match("open window"))
assert(not pcall(mv.load_image, "a\0b.png"))
assert(not pcall(mv.image_size, {}))
assert(not pcall(mv.draw_image, {}, 0, 0))
