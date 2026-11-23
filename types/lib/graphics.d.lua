-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0
--
-- Graphics and collision types for the Lua Language Server (LuaLS).

---@meta

---Explicit camera returned by `camera_create`.
---@class MvCamera
---@field set fun(self: MvCamera, x: number, y: number)
---@field move fun(self: MvCamera, dx: number, dy: number)
---@field position fun(self: MvCamera):number, number
---@field follow fun(self: MvCamera, target: MvProcess)
---@field destroy fun(self: MvCamera)

---Options for `camera_create`.
---@class MvCameraCreateOptions
---@field viewport MvRegion? Region descriptor used as the camera viewport.

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
---@field add_rect fun(self: MvCollisionWorld, rect: MvWorldRect):integer
---@field overlap fun(self: MvCollisionWorld, shape: MvCollisionShape, transform?: MvCollisionTransform):integer|nil
---@field sweep fun(self: MvCollisionWorld, shape: MvCollisionShape, transform: MvCollisionTransform|nil, dx: number, dy: number):MvSweepResult
---@field collision fun(self: MvCollisionWorld, process: MvProcess, options?: MvWorldCollisionOptions):integer|nil
---@field move fun(self: MvCollisionWorld, process: MvProcess, dx: number, dy: number):MvMoveResult

---Rectangle added with `world:add_rect`.
---@class MvWorldRect
---@field x number Left edge. Required.
---@field y number Top edge. Required.
---@field width number Rectangle width. Required.
---@field height number Rectangle height. Required.

---Options for `world:collision`.
---@class MvWorldCollisionOptions
---@field x number? Horizontal position override.
---@field y number? Vertical position override.

---Process graphic returned by `rect` and `circle`.
---@class MvGraphic

---Region descriptor returned by `region`.
---@class MvRegion

---Scroll descriptor returned by `scroll`.
---@class MvScroll

---Animation descriptor returned by `animation` (atlas or frame list).
---@class MvAnimation
---@field sample fun(self: MvAnimation, elapsed_ms: number):MvGraphic

---Options for `rect` and `circle`.
---@class MvGraphicOptions
---@field color MvColor Fill color. Default: opaque white.
---@field pivot "center"|"top-left"? Pivot position (`rect` only). Default `"center"`.

---Options for `animation` with a frame list.
---@class MvAnimationOptions
---@field fps number? Playback speed in frames per second (1/3600..1000). Default 8.
---@field loop boolean? Loop the animation. Default true.
