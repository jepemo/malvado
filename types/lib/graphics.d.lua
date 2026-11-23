-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0
--
-- Graphics and collision types for the Lua Language Server (LuaLS).

---@meta

---Explicit camera returned by `camera_create`.
---@class MvCamera

---Options for `camera_create`.
---@class MvCameraCreateOptions
---@field x? number World x of the camera. Defaults to 0.
---@field y? number World y of the camera. Defaults to 0.
---@field viewport? MvRegion Screen-space viewport. Defaults to the full window.

---Collision shape returned by `collision.rect` and `collision.circle`.
---@class MvCollisionShape

---Collision transform table accepted by `collision.overlap` and world queries.
---@class MvCollisionTransform
---@field x number? Horizontal offset. Default 0.
---@field y number? Vertical offset. Default 0.
---@field angle number? Rotation in degrees. Default 0.
---@field scale number? Scale factor. Default 1.

---Options for the shape constructors `collision.rect` and `collision.circle`.
---@class MvShapeOptions
---@field offset_x number? Horizontal collider offset. Default 0.
---@field offset_y number? Vertical collider offset. Default 0.

---Static collision world returned by `world_create`.
---@class MvCollisionWorld

---Rectangle added with `world_add_rect`.
---@class MvWorldRect
---@field x number Left edge. Required.
---@field y number Top edge. Required.
---@field width number Rectangle width. Required.
---@field height number Rectangle height. Required.

---Options for `world_collision`.
---@class MvWorldCollisionOptions
---@field x number? Horizontal position override.
---@field y number? Vertical position override.

---Process graphic returned by `rect` and `circle`.
---@class MvGraphic

---Region descriptor returned by `region`.
---@class MvRegion

---Options for a screen-space region.
---@class MvRegionOptions
---@field x integer Left edge in window pixels.
---@field y integer Top edge in window pixels.
---@field width integer Positive width.
---@field height integer Positive height.

---Scroll descriptor returned by `scroll`.
---@class MvScroll

---Animation descriptor returned by `animation` (atlas or frame list).
---@class MvAnimation

---Options for a reusable rectangle graphic.
---@class MvRectOptions
---@field width number Positive width in local pixels. Fractions are allowed.
---@field height number Positive height in local pixels. Fractions are allowed.
---@field color? MvColor Fill color. Defaults to opaque white.
---@field pivot? "center"|"top-left" Origin on the rectangle. Defaults to "center".

---Options for a reusable circle graphic with a center pivot.
---@class MvCircleOptions
---@field radius number Positive radius in local pixels. Fractions are allowed.
---@field color? MvColor Fill color. Defaults to opaque white.

---Options for an immediate rectangle.
---@class MvDrawRectOptions
---@field x integer Left edge in world pixels.
---@field y integer Top edge in world pixels.
---@field width integer Positive width. The edge at x + width is exclusive.
---@field height integer Positive height. The edge at y + height is exclusive.
---@field color? MvColor Command color. Defaults to the current drawing color.
---@field filled? boolean Use false for an outline. Defaults to true.

---Options for an immediate circle.
---@class MvDrawCircleOptions
---@field x integer Center x in world pixels.
---@field y integer Center y in world pixels.
---@field radius integer Positive radius in world pixels.
---@field color? MvColor Command color. Defaults to the current drawing color.
---@field filled? boolean Use false for an outline. Defaults to true.

---Options for an immediate line.
---@class MvDrawLineOptions
---@field x1 integer Start x in world pixels.
---@field y1 integer Start y in world pixels.
---@field x2 integer End x in world pixels.
---@field y2 integer End y in world pixels.
---@field color? MvColor Command color. Defaults to the current drawing color.
---@field thickness? number Positive thickness in world pixels. Defaults to 1.

---Options for `animation` with a frame list.
---@class MvAnimationOptions
---@field fps number? Playback speed in frames per second (1/3600..1000). Default 8.
---@field loop boolean? Loop the animation. Default true.
