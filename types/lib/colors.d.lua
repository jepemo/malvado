-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

---@meta

---@alias MvColorRGB [integer, integer, integer]
---@alias MvColorRGBA [integer, integer, integer, integer]

---@class MvNamedColor
---@field r integer Red from 0 to 255.
---@field g integer Green from 0 to 255.
---@field b integer Blue from 0 to 255.
---@field a? integer Alpha from 0 to 255. Defaults to 255.

---Read-only native palette value. Both sets of fields expose the same channels.
---@class MvPaletteColor: userdata
---@field r integer Red.
---@field g integer Green.
---@field b integer Blue.
---@field a integer Alpha.
---@field [1] integer Red.
---@field [2] integer Green.
---@field [3] integer Blue.
---@field [4] integer Alpha.

---A palette color or an RGB or RGBA table. Channels are integers from 0 to 255.
---Alpha defaults to 255. Use either positional channels or named fields.
---@alias MvColor MvColorRGB|MvColorRGBA|MvNamedColor|MvPaletteColor

---@class MvColorPalette
---@field BLACK MvPaletteColor RGBA (0, 0, 0, 255).
---@field WHITE MvPaletteColor RGBA (255, 255, 255, 255).
---@field TRANSPARENT MvPaletteColor RGBA (0, 0, 0, 0).
---@field GRAY MvPaletteColor RGBA (128, 128, 128, 255).
---@field LIGHT_GRAY MvPaletteColor RGBA (211, 211, 211, 255).
---@field DARK_GRAY MvPaletteColor RGBA (64, 64, 64, 255).
---@field RED MvPaletteColor RGBA (255, 0, 0, 255).
---@field GREEN MvPaletteColor RGBA (0, 255, 0, 255).
---@field BLUE MvPaletteColor RGBA (0, 0, 255, 255).
---@field YELLOW MvPaletteColor RGBA (255, 255, 0, 255).
---@field CYAN MvPaletteColor RGBA (0, 255, 255, 255).
---@field MAGENTA MvPaletteColor RGBA (255, 0, 255, 255).
---@field ORANGE MvPaletteColor RGBA (255, 165, 0, 255).
---@field PURPLE MvPaletteColor RGBA (128, 0, 128, 255).
---@field PINK MvPaletteColor RGBA (255, 192, 203, 255).
---@field BROWN MvPaletteColor RGBA (165, 42, 42, 255).
