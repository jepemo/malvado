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
---@field size fun(self: MvImage):integer, integer

---Sprite atlas loaded with `load_asset`, `load_atlas` or `load_aseprite`.
---@class MvAtlas
local Atlas = {}

---@param id integer
---@return MvSpriteInfo
function Atlas:sprite_info(id) end

---@param name string
---@return integer|nil
function Atlas:sprite_id(name) end

---@param name string
---@param elapsed_ms number
---@param options? MvNamedAnimationOptions
---@return integer
function Atlas:animation_frame(name, elapsed_ms, options) end

---@param name string
---@return integer[]|nil
function Atlas:animation_color(name) end

---@param name string
---@return integer|nil
function Atlas:animation_repeats(name) end

---@return integer[]|nil
function Atlas:pivot_color() end

---@param id integer
---@return MvGraphic
function Atlas:graph(id) end

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
---@field count fun(self: MvFpg):integer
---@field id_at fun(self: MvFpg, index: integer):integer|nil
---@field palette fun(self: MvFpg):string
---@field gamma fun(self: MvFpg):string
---@field sprite fun(self: MvFpg, id: integer):MvFpgSprite|nil
---@field point fun(self: MvFpg, id: integer, index: integer):MvFpgPoint|nil
---@field rgba fun(self: MvFpg, id: integer, transparent_zero?: boolean):string|nil
---@field image fun(self: MvFpg, id: integer, transparent_zero?: boolean):MvImage|nil
---@field png fun(self: MvFpg, id: integer, transparent_zero?: boolean):string|nil
---@field json fun(self: MvFpg, transparent_zero?: boolean):string
---@field graph fun(self: MvFpg, id: integer):MvGraphic

---FPG manifest loaded with `load_asset` or `fpg.parse_manifest`.
---@class MvFpgManifest
---@field count fun(self: MvFpgManifest):integer
---@field id_at fun(self: MvFpgManifest, index: integer):integer|nil
---@field palette fun(self: MvFpgManifest):string
---@field gamma fun(self: MvFpgManifest):string
---@field sprite fun(self: MvFpgManifest, id: integer):MvManifestSprite|nil
---@field point fun(self: MvFpgManifest, id: integer, index: integer):MvFpgPoint|nil
---@field transparent_zero fun(self: MvFpgManifest):boolean
---@field graph fun(self: MvFpgManifest, id: integer):MvGraphic

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
