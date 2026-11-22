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
---@field graph fun(self: MvAtlas, id: integer):MvGraphic
---@field animation fun(self: MvAtlas, name: string, options?: MvAnimationOptions):MvAnimation

---FPG library loaded with `load_asset` or `fpg.parse`.
---@class MvFpg
---@field count fun(self: MvFpg):integer
---@field id_at fun(self: MvFpg, index: integer):integer|nil
---@field palette fun(self: MvFpg):string
---@field gamma fun(self: MvFpg):string
---@field sprite fun(self: MvFpg, id: integer):MvFpgSprite|nil
---@field point fun(self: MvFpg, id: integer, index: integer):MvFpgPoint|nil
---@field rgba fun(self: MvFpg, id: integer, transparent_zero?: boolean):string
---@field image fun(self: MvFpg, id: integer, transparent_zero?: boolean):MvImage|nil
---@field png fun(self: MvFpg, id: integer, transparent_zero?: boolean):string|nil
---@field json fun(self: MvFpg, transparent_zero?: boolean):string
---@field graph fun(self: MvFpg, id: integer):MvGraphic
---@field animation fun(self: MvFpg, sequence: string|integer[], options?: MvAnimationOptions):MvAnimation

---FPG manifest loaded with `load_asset` or `fpg.parse_manifest`.
---@class MvFpgManifest
---@field count fun(self: MvFpgManifest):integer
---@field id_at fun(self: MvFpgManifest, index: integer):integer|nil
---@field transparent_zero fun(self: MvFpgManifest):boolean
---@field palette fun(self: MvFpgManifest):string
---@field gamma fun(self: MvFpgManifest):string
---@field sprite fun(self: MvFpgManifest, id: integer):MvFpgSprite|nil
---@field point fun(self: MvFpgManifest, id: integer, index: integer):MvFpgPoint|nil
---@field graph fun(self: MvFpgManifest, id: integer):MvGraphic
---@field animation fun(self: MvFpgManifest, name: string, options?: MvAnimationOptions):MvAnimation

---Sprite info table returned by `fpg:sprite` and `sprite_info`.
---@class MvFpgSprite
---@field id integer
---@field width integer
---@field height integer
---@field point_count integer
---@field name_raw string
---@field filename_raw string
---@field pixels string|nil Raw RGBA bytes when available.

---Control point returned by `fpg:point`.
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
