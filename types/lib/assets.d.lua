-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0
--
-- Drawing and asset types for the Lua Language Server (LuaLS).

---@meta

---PNG pivot options: the asset must be a PNG image.
---@class MvLoadAssetPivotOptions
---@field pivot string Pivot position: "center" or "top-left".

---Options for `load_asset`.
---@class MvLoadAssetOptions
---@field format "png"|"fpg"|"atlas"|"aseprite"|"fpg-manifest"? Force an asset format. Default: detected from the file.
---@field transparent_zero boolean? Treat palette index 0 as transparent (FPG only). Default true.
---@field pivot "center"|"top-left"? Pivot for PNG images. Default: keep the loaded pivot.

---Image loaded with `load_image` or `load_asset`.
---@class MvImage

---Sprite atlas loaded with `load_asset`, `load_atlas` or `load_aseprite`.
---@class MvAtlas

---Source rectangle in image pixels.
---@class MvImageSource
---@field x integer Non-negative left edge.
---@field y integer Non-negative top edge.
---@field width integer Positive width.
---@field height integer Positive height.

---A point relative to the selected source rectangle.
---@class MvPoint
---@field x number
---@field y number

---@alias MvImagePivot "top-left"|"center"|MvPoint

---@class MvDrawImageOptions
---@field image MvImage
---@field x number Horizontal position of the pivot.
---@field y number Vertical position of the pivot.
---@field source? MvImageSource Defaults to the full image.
---@field pivot? MvImagePivot Defaults to "top-left".
---@field scale? number Positive uniform scale. Defaults to 1.
---@field angle? number Clockwise degrees. Defaults to 0.

---@class MvDrawSpriteOptions
---@field atlas MvAtlas
---@field id integer Positive sprite ID in the atlas.
---@field x number Horizontal position of the metadata pivot.
---@field y number Vertical position of the metadata pivot.
---@field scale? number Positive uniform scale. Defaults to 1.
---@field angle? number Clockwise degrees. Defaults to 0.

---FPG library loaded with `load_asset` or `fpg.parse`.
---@class MvFpg

---FPG manifest loaded with `load_asset` or `fpg.parse_manifest`.
---@class MvFpgManifest

---Metadata shared by binary FPG and manifest sprite entries.
---@class MvFpgSpriteMetadata
---@field id integer
---@field width integer
---@field height integer
---@field point_count integer
---@field name_raw string
---@field filename_raw string

---Sprite returned by `fpg_sprite`.
---@class MvFpgSprite : MvFpgSpriteMetadata
---@field pixels string Raw palette indices, one byte per pixel.

---Sprite returned by `manifest_sprite`.
---@class MvManifestSprite : MvFpgSpriteMetadata
---@field image string Relative PNG filename.

---Control point returned by `fpg_point` and `manifest_point`.
---@class MvFpgPoint
---@field x integer
---@field y integer
---@field defined boolean

---Sprite metadata returned by `sprite_info`.
---@class MvSpriteInfo
---@field name string
---@field id integer
---@field x integer
---@field y integer
---@field width integer
---@field height integer
---@field pivot_x integer
---@field pivot_y integer
---@field source_width integer
---@field source_height integer
---@field trim_x integer
---@field trim_y integer
