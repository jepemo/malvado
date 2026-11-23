-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0
--
-- Type definitions for the Lua Language Server (LuaLS).
-- These files describe the public Malvado Lua API without executing code.

---@meta

---Built-in horizontal text alignment.
---@alias MvTextAlign "left"|"center"|"right"

---Font descriptor returned by `font` and `load_font`.
---A font describes typography and its default size. Colors belong to text.
---@class MvFont
---@field size integer Default font size in pixels (1..1024).
---@field resource userdata|nil Native font resource for loaded TTF/OTF fonts.

---@class MvFontOptions
---@field size integer Font size in pixels (1..1024). Required.

---@class MvLoadFontOptions
---@field path string Font file path. Required.
---@field size integer Base font size in pixels (1..1024). Required.
---@field extra_glyphs string? Extra Unicode glyphs to load, as a UTF-8 string.

---@class MvWriteOptions
---@field font MvFont Font descriptor. Required.
---@field text string Text content. Required.
---@field x integer Horizontal screen position. Required.
---@field y integer Vertical screen position. Required.
---@field align MvTextAlign? Horizontal alignment. Default: `"left"`.
---@field color MvColor? RGBA color. Default: opaque white.
---@field size integer? Font size override (1..1024). Default: the font's size.

---@class MvChangeTextOptions
---@field text string? Replacement text.
---@field x integer? New horizontal position.
---@field y integer? New vertical position.
---@field align MvTextAlign? New horizontal alignment.
---@field color MvColor? New RGBA color. Copied on assignment.
---@field size integer? New font size (1..1024).
---@field font MvFont? Replacement font descriptor.

---@class MvDrawTextOptions
---@field text string Text content. Required.
---@field x integer Horizontal position. Required.
---@field y integer Vertical position. Required.
---@field size integer? Font size in pixels (default 20).
---@field color MvColor? RGBA color for this draw only (default opaque white).
