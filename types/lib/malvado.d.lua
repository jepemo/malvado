-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0
--
-- Malvado Lua module definition for the Lua Language Server (LuaLS).
-- This file describes the public API without executing runtime functionality.
--
-- Use the namespaced API:
--   local mv = require("malvado")
--   mv.write { ... }
--
-- Or install the same functions as globals (opt-in):
--   require("malvado").import_globals()
--   write { ... }

---@meta malvado

---@alias MvProcessConstructor fun(fields?: table, options?: MvSpawnOptions):MvProcess

---@class MvProcessNamespace
---@field __call fun(_self: MvProcessNamespace, body: fun(self: MvProcess), options?: MvProcessDefineOptions):MvProcessConstructor Define a process. Returns a constructor; calling the constructor spawns the process and returns its handle. The body runs as a coroutine and receives `self`. Use `frame()` to yield until the next activation.
---@field define fun(body: fun(self: MvProcess), options?: MvProcessDefineOptions):MvProcessConstructor
---@field spawn fun(constructor: MvProcessConstructor, fields?: table, options?: MvSpawnOptions):MvProcess
---@field find fun(constructor: MvProcessConstructor):MvProcess[]
---@field children fun(handle: MvProcess, options?: MvTreeQueryOptions):MvProcess[]
---@field descendants fun(handle: MvProcess, options?: MvTreeQueryOptions):MvProcess[]
---@field snapshot fun():MvSnapshot
---@field frame fun()
---@field signal fun(handle: MvProcess, signal: MvSignal)
---@field signal_tree fun(handle: MvProcess, signal: MvSignal, options?: MvTreeSignalOptions)
---@field set_priority (fun(priority: integer))|(fun(handle: MvProcess, priority: integer)) The one-argument form requires the scheduled process coroutine.
---@field get_priority fun(handle: MvProcess):integer
---@field kill (fun())|(fun(handle: MvProcess):boolean) The no-argument form requires the scheduled process coroutine. Self-kill stops it immediately.
---@field alive fun(handle: MvProcess):boolean
---@field kill_tree fun(handle: MvProcess, options?: MvTreeSignalOptions)
---@field status fun(handle: MvProcess):MvProcessStatusName
---@field send fun(handle: MvProcess, message: any)
---@field recv (fun():any)|(fun(handle: MvProcess):any) Read the next message or nil when empty. The no-argument form requires the scheduled process coroutine.
---@field collision fun(source: MvProcess, target: MvProcess):MvProcess|nil
---@field collisions fun(source: MvProcess, target: MvProcess|MvProcessConstructor):MvProcess[]
---@field SIGNAL_SLEEP "sleep"
---@field SIGNAL_FREEZE "freeze"
---@field SIGNAL_WAKEUP "wakeup"
---@field SIGNAL_KILL "kill"
---@field STATUS_RUNNING "running"
---@field STATUS_SLEEPING "sleeping"
---@field STATUS_FROZEN "frozen"
---@field STATUS_DEAD "dead"

---@class MvInputNamespace
---@field key fun(name: MvKey):boolean
---@field key_pressed fun(name: MvKey):boolean
---@field key_released fun(name: MvKey):boolean
---@field mouse_position fun():integer, integer
---@field mouse_button fun(button: MvMouseButton):boolean
---@field mouse_pressed fun(button: MvMouseButton):boolean
---@field mouse_released fun(button: MvMouseButton):boolean
---@field pad_connected fun(slot: integer):boolean
---@field pad_button fun(slot: integer, name: MvPadButton):boolean
---@field pad_pressed fun(slot: integer, name: MvPadButton):boolean
---@field pad_released fun(slot: integer, name: MvPadButton):boolean
---@field pad_axis fun(slot: integer, name: MvPadAxis):number
---@field text_input_next fun():string|nil
---@field text_input_set_keyboard fun(enabled: boolean):boolean
---@field KEY_A "a"
---@field KEY_B "b"
---@field KEY_C "c"
---@field KEY_D "d"
---@field KEY_E "e"
---@field KEY_F "f"
---@field KEY_G "g"
---@field KEY_H "h"
---@field KEY_I "i"
---@field KEY_J "j"
---@field KEY_K "k"
---@field KEY_L "l"
---@field KEY_M "m"
---@field KEY_N "n"
---@field KEY_O "o"
---@field KEY_P "p"
---@field KEY_Q "q"
---@field KEY_R "r"
---@field KEY_S "s"
---@field KEY_T "t"
---@field KEY_U "u"
---@field KEY_V "v"
---@field KEY_W "w"
---@field KEY_X "x"
---@field KEY_Y "y"
---@field KEY_Z "z"
---@field KEY_0 "0"
---@field KEY_1 "1"
---@field KEY_2 "2"
---@field KEY_3 "3"
---@field KEY_4 "4"
---@field KEY_5 "5"
---@field KEY_6 "6"
---@field KEY_7 "7"
---@field KEY_8 "8"
---@field KEY_9 "9"
---@field KEY_ESCAPE "escape"
---@field KEY_SPACE "space"
---@field KEY_LEFT "left"
---@field KEY_RIGHT "right"
---@field KEY_UP "up"
---@field KEY_DOWN "down"
---@field KEY_ENTER "enter"
---@field KEY_TAB "tab"
---@field KEY_BACKSPACE "backspace"
---@field MOUSE_LEFT "left"
---@field MOUSE_MIDDLE "middle"
---@field MOUSE_RIGHT "right"
---@field PAD_A "a"
---@field PAD_B "b"
---@field PAD_X "x"
---@field PAD_Y "y"
---@field PAD_UP "up"
---@field PAD_DOWN "down"
---@field PAD_LEFT "left"
---@field PAD_RIGHT "right"
---@field PAD_LB "lb"
---@field PAD_RB "rb"
---@field PAD_BACK "back"
---@field PAD_START "start"
---@field PAD_LEFT_X "left_x"
---@field PAD_LEFT_Y "left_y"
---@field PAD_RIGHT_X "right_x"
---@field PAD_RIGHT_Y "right_y"
---@field PAD_LEFT_TRIGGER "left_trigger"
---@field PAD_RIGHT_TRIGGER "right_trigger"

---@class MvGraphicsNamespace
---@field rect fun(options: MvRectOptions):MvGraphic
---@field circle fun(options: MvCircleOptions):MvGraphic
---@field region fun(options: MvRegionOptions):MvRegion
---@field scroll fun(x: number, y: number):MvScroll
---@field animation_elapsed (fun():number|nil)|(fun(handle: MvProcess):number|nil) The no-argument form requires the scheduled process coroutine.
---@field animation_finished (fun():boolean)|(fun(handle: MvProcess):boolean) The no-argument form requires the scheduled process coroutine.

---@class MvCollisionNamespace
---@field __call fun(_self: MvCollisionNamespace, target: MvProcessConstructor):MvProcess|nil
---@field circle fun(options: MvCollisionCircleOptions):MvCollisionShape
---@field rect fun(options: MvCollisionRectOptions):MvCollisionShape
---@field overlap fun(shape_a: MvCollisionShape, shape_b: MvCollisionShape, options?: MvCollisionOverlapOptions):boolean

---@class MvFpgNamespace
---@field parse fun(bytes: string):MvFpg
---@field parse_manifest fun(json: string):MvFpgManifest

---The `malvado` module.
---@class malvado
---@field process MvProcessNamespace
---@field input MvInputNamespace
---@field graphics MvGraphicsNamespace
---@field colors MvColorPalette
---@field collision MvCollisionNamespace
---@field fpg MvFpgNamespace
local malvado = {}

---Spawn a process from a constructor with explicit fields and options.
---@param constructor MvProcessConstructor
---@param fields? table
---@param options? MvSpawnOptions
---@return MvProcess handle
function malvado.spawn(constructor, fields, options) end

---Find all live processes of one type.
---@param constructor MvProcessConstructor
---@return MvProcess[] handles
function malvado.find(constructor) end

---List direct children of a process.
---@param handle MvProcess
---@param options? MvTreeQueryOptions
---@return MvProcess[] children
function malvado.children(handle, options) end

---List all descendants of a process.
---@param handle MvProcess
---@param options? MvTreeQueryOptions
---@return MvProcess[] descendants
function malvado.descendants(handle, options) end

---Inspect every live process.
---@return MvSnapshot snapshot
function malvado.snapshot() end

---Yield the current process until the next tick.
---Must be called inside a process body.
function malvado.frame() end

---Send a signal: "sleep", "freeze", "wakeup" or "kill".
---Kill removes the process and its owned texts.
---@param handle MvProcess
---@param signal MvSignal
function malvado.signal(handle, signal) end

---Send a signal to a process tree.
---@param handle MvProcess
---@param signal MvSignal
---@param options? MvTreeSignalOptions
function malvado.signal_tree(handle, signal, options) end

---Set an integer priority. The one-argument form requires the scheduled process coroutine.
---@overload fun(priority: integer)
---@param handle MvProcess
---@param priority integer
function malvado.set_priority(handle, priority) end

---Read the scheduling priority of a process.
---@param handle MvProcess
---@return integer priority
function malvado.get_priority(handle) end

---Kill a process. With no handle, kill the currently scheduled process from its own coroutine.
---Self-kill stops the process immediately and does not return to it.
---@overload fun()
---@param handle MvProcess
---@return boolean killed Whether the target was live.
function malvado.kill(handle) end

---Check whether a process handle is still alive.
---@param handle MvProcess
---@return boolean alive
function malvado.alive(handle) end

---Kill a process and its whole tree.
---@param handle MvProcess
---@param options? MvTreeSignalOptions
function malvado.kill_tree(handle, options) end

---Read the same status string as `handle.status`, including "dead" after termination.
---@param handle MvProcess
---@return MvProcessStatusName status
function malvado.status(handle) end

---Queue a message for a process. The recipient can read it with `recv()`.
---@param handle MvProcess
---@param message any
function malvado.send(handle, message) end

---Remove and return the next queued message, or nil when the queue is empty. False is a valid message.
---With no argument, read from the currently scheduled process coroutine.
---@overload fun():any
---@param handle MvProcess
---@return any message
function malvado.recv(handle) end

---Run the game loop. Call once with the entry process.
---
---```lua
---start(function() Player {} end, {width = 640, height = 480})
---```
---@param body fun()
---@param options? MvStartOptions
function malvado.start(body, options) end

---Set the default cadence for future Lua processes in the running game.
---Existing processes keep their current cadence.
---@param fps integer Activations per 60 eligible logical ticks (0..60). Both 0 and 60 run every tick.
function malvado.set_fps(fps) end

---Create a rectangle process graphic.
---@param options MvRectOptions
---@return MvGraphic
function malvado.rect(options) end

---Create a circle process graphic.
---@param options MvCircleOptions
---@return MvGraphic
function malvado.circle(options) end

---Create a fixed screen-space clip for a process.
---@param options MvRegionOptions
---@return MvRegion
function malvado.region(options) end

---Create an absolute offset that is subtracted from one process's drawing.
---@param x number
---@param y number
---@return MvScroll
function malvado.scroll(x, y) end

---Check whether two axis-aligned rectangles overlap.
---@param a MvWorldRect
---@param b MvWorldRect
---@return boolean overlaps
function malvado.rect_overlaps(a, b) end

---Check whether a rectangle contains a point.
---@param rect MvWorldRect
---@param x number
---@param y number
---@return boolean contains
function malvado.rect_contains(rect, x, y) end

---Create a managed explicit camera. An empty table selects (0, 0) and the full logical window.
---@param options MvCameraCreateOptions
---@return MvCamera camera
function malvado.camera_create(options) end

---Position the main camera: `camera_set(x, y)`.
---Position an explicit camera: `camera_set(camera, x, y)`.
---@overload fun(x: number, y: number)
---@param camera MvCamera
---@param x number
---@param y number
function malvado.camera_set(camera, x, y) end

---Move the main camera: `camera_move(dx, dy)`.
---Move an explicit camera: `camera_move(camera, dx, dy)`.
---@overload fun(dx: number, dy: number)
---@param camera MvCamera
---@param dx number
---@param dy number
function malvado.camera_move(camera, dx, dy) end

---Follow a live process with the main camera, or with an explicit camera.
---@overload fun(camera: MvCamera, target: MvProcess)
---@param target MvProcess
function malvado.camera_follow(target) end

---Read a camera position. Without arguments, reads the main camera.
---@param camera? MvCamera
---@return number x
---@return number y
function malvado.camera_position(camera) end

---Close an explicit camera. Repeated calls for the same camera are safe.
---@param camera MvCamera
function malvado.camera_destroy(camera) end

---Create a static collision world.
---@return MvCollisionWorld world
function malvado.world_create() end

---Add a rectangle to the world and return its ID.
---@param world MvCollisionWorld
---@param rect MvWorldRect
---@return integer id
function malvado.world_add_rect(world, rect) end

---Return the first overlapping rectangle ID, or nil.
---@param world MvCollisionWorld
---@param shape MvCollisionShape
---@param transform? MvCollisionTransform
---@return integer|nil id
function malvado.world_overlap(world, shape, transform) end

---Sweep a shape through the world without moving it.
---@param world MvCollisionWorld
---@param shape MvCollisionShape
---@param options MvWorldSweepOptions
---@return MvSweepResult result
function malvado.world_sweep(world, shape, options) end

---Test a live process collider at its current or hypothetical position.
---The contextual form uses the scheduled process and its `world` field.
---@overload fun(options?: MvWorldCollisionOptions):integer|nil
---@param world MvCollisionWorld
---@param process MvProcess
---@param options? MvWorldCollisionOptions
---@return integer|nil id
function malvado.world_collision(world, process, options) end

---Move the current process through the world selected by its `world` field
---and report the collision result.
---@overload fun(world: MvCollisionWorld, process: MvProcess, dx: number, dy: number):MvMoveResult
---@param dx number
---@param dy number
---@return MvMoveResult result
function malvado.world_move(dx, dy) end

---Load an image, atlas, FPG or Aseprite export from a path.
---
---```lua
---local image = load_asset("assets/logo.png")
---local atlas = load_asset("assets/sheet.json")
---```
---The `format` option selects the loader. With `pivot` the asset must be a PNG
---image. Without options the format is detected from the file.
---@overload fun(path: string, options: MvLoadAssetPivotOptions):MvImage
---@overload fun(path: string, options: {format: "png"}):MvImage
---@overload fun(path: string, options: {format: "fpg"}):MvFpg
---@overload fun(path: string, options: {format: "fpg-manifest"}):MvFpgManifest
---@overload fun(path: string, options: {format: "atlas"|"aseprite"}):MvAtlas
---@param path string
---@param options? MvLoadAssetOptions
---@return MvImage|MvAtlas|MvFpg|MvFpgManifest
function malvado.load_asset(path, options) end

---Set the asset directory relative to the game directory. Use nil or "." to reset.
---@param path? string
function malvado.set_assets_dir(path) end

---Return the current absolute asset directory.
---@return string directory
function malvado.get_assets_dir() end

---Load a PNG image. Requires an open window.
---@param path string
---@return MvImage image
function malvado.load_image(path) end

---Read an image size in pixels.
---@param image MvImage
---@return integer width
---@return integer height
function malvado.image_size(image) end

---Queue an image draw until the process next activates.
---Must be called inside a process.
---@param options MvDrawImageOptions
function malvado.draw_image(options) end

---Parse a sprite atlas from an image and its JSON metadata.
---@param image MvImage
---@param json string
---@return MvAtlas atlas
function malvado.parse_atlas(image, json) end

---Parse a sprite atlas from an Aseprite JSON export.
---@param image MvImage
---@param json string
---@return MvAtlas atlas
function malvado.parse_aseprite(image, json) end

---Read an independent sprite metadata table. Returns nil when missing.
---@param atlas MvAtlas
---@param id_or_name integer|string Exact sprite name or positive ID.
---@return MvSpriteInfo|nil info
function malvado.atlas_sprite_info(atlas, id_or_name) end

---Find the sprite ID for an atlas sprite name. Returns nil when missing.
---@param atlas MvAtlas
---@param name string
---@return integer|nil id
function malvado.atlas_sprite_id(atlas, name) end

---Compute the atlas animation frame for an elapsed time in milliseconds.
---@param atlas MvAtlas
---@param name string
---@param elapsed_ms number Finite milliseconds from 0 to 1e12.
---@param options? MvNamedAnimationOptions
---@return integer sprite_id
function malvado.atlas_animation_frame(atlas, name, elapsed_ms, options) end

---Read the imported color of an atlas animation tag. Returns nil when missing.
---@param atlas MvAtlas
---@param name string
---@return integer[]|nil color
function malvado.atlas_animation_color(atlas, name) end

---Read the transparent pivot color of an atlas. Returns nil when missing.
---@param atlas MvAtlas
---@return integer[]|nil color
function malvado.atlas_pivot_color(atlas) end

---Read the imported repeat count of an atlas animation tag. Returns nil when missing.
---@param atlas MvAtlas
---@param name string
---@return integer|nil repeats
function malvado.atlas_animation_repeats(atlas, name) end

---Queue an atlas sprite draw with its metadata pivot until the process next activates.
---Must be called inside a process.
---@param options MvDrawSpriteOptions
function malvado.draw_sprite(options) end

---Select a static graphic to assign to `self.graph`. With one argument, the
---graphic comes from the current process's `file` field.
---@overload fun(id_or_name: integer|string):MvGraphic
---@overload fun(library: MvAtlas, id_or_name: integer|string):MvGraphic
---@param library MvFpg|MvFpgManifest
---@param id integer
---@return MvGraphic
function malvado.graph(library, id) end
malvado.graphics.graph = malvado.graph

---Create an animation descriptor from an atlas animation or a frame list.
---
---```lua
---self.graph = animation(atlas, "walk")
---self.graph = animation("walk")        -- uses the process's `file` field
---self.graph = animation({1, 2, 3}, {fps = 12})
---```
---@overload fun(name: string, options?: MvNamedAnimationOptions):MvAnimation
---@overload fun(library: MvAtlas, name: string, options?: MvNamedAnimationOptions):MvAnimation
---@overload fun(library: MvAtlas|MvFpg|MvFpgManifest, sequence: integer[], options?: MvAnimationOptions):MvAnimation
---@param sequence integer[]
---@param options? MvAnimationOptions
---@return MvAnimation
function malvado.animation(sequence, options) end

malvado.graphics.animation = malvado.animation

---Sample a reusable animation descriptor at a time in milliseconds.
---This does not change any process playback clock.
---@param animation MvAnimation
---@param elapsed_ms number
---@return MvGraphic graphic
function malvado.animation_sample(animation, elapsed_ms) end

---Read elapsed playback in milliseconds. Returns nil without an active animation.
---The no-argument form requires the currently scheduled process coroutine.
---@overload fun():number|nil
---@param handle MvProcess
---@return number|nil elapsed_ms
function malvado.animation_elapsed(handle) end

---Check whether a non-looping animation has played through its final frame.
---The no-argument form requires the currently scheduled process coroutine.
---@overload fun():boolean
---@param handle MvProcess
---@return boolean finished
function malvado.animation_finished(handle) end

---Return the number of sprites.
---@param fpg MvFpg
---@return integer count
function malvado.fpg_count(fpg) end

---Return the sprite ID at a one-based index, or nil.
---@param fpg MvFpg
---@param index integer
---@return integer|nil id
function malvado.fpg_id_at(fpg, index) end

---Return the 768 original six-bit RGB bytes.
---@param fpg MvFpg
---@return string palette
function malvado.fpg_palette(fpg) end

---Return the 576 original gamma bytes.
---@param fpg MvFpg
---@return string gamma
function malvado.fpg_gamma(fpg) end

---Read a sprite entry, or return nil if it is missing.
---@param fpg MvFpg
---@param id integer
---@return MvFpgSprite|nil sprite
function malvado.fpg_sprite(fpg, id) end

---Read a zero-based control point, or return nil if it is missing.
---@param fpg MvFpg
---@param id integer
---@param index integer
---@return MvFpgPoint|nil point
function malvado.fpg_point(fpg, id, index) end

---Return RGBA bytes, or nil if the sprite is missing.
---@param fpg MvFpg
---@param id integer
---@param options? MvFpgConversionOptions
---@return string|nil bytes
function malvado.fpg_rgba(fpg, id, options) end

---Create an independent image in an open window, or return nil if the sprite is missing.
---@param fpg MvFpg
---@param id integer
---@param options? MvFpgConversionOptions
---@return MvImage|nil image
function malvado.fpg_image(fpg, id, options) end

---Encode an indexed PNG, or return nil if the sprite is missing.
---@param fpg MvFpg
---@param id integer
---@param options? MvFpgConversionOptions
---@return string|nil bytes
function malvado.fpg_png(fpg, id, options) end

---Encode the sprite metadata as a JSON manifest.
---@param fpg MvFpg
---@param options? MvFpgConversionOptions
---@return string json
function malvado.fpg_json(fpg, options) end

---Return the number of sprites.
---@param manifest MvFpgManifest
---@return integer count
function malvado.manifest_count(manifest) end

---Return the sprite ID at a one-based index, or nil.
---@param manifest MvFpgManifest
---@param index integer
---@return integer|nil id
function malvado.manifest_id_at(manifest, index) end

---Return the 768 original six-bit RGB bytes.
---@param manifest MvFpgManifest
---@return string palette
function malvado.manifest_palette(manifest) end

---Return the 576 original gamma bytes.
---@param manifest MvFpgManifest
---@return string gamma
function malvado.manifest_gamma(manifest) end

---Read a sprite entry, or return nil if it is missing.
---@param manifest MvFpgManifest
---@param id integer
---@return MvManifestSprite|nil sprite
function malvado.manifest_sprite(manifest, id) end

---Read a zero-based control point, or return nil if it is missing.
---@param manifest MvFpgManifest
---@param id integer
---@param index integer
---@return MvFpgPoint|nil point
function malvado.manifest_point(manifest, id, index) end

---Read the palette index zero transparency flag.
---@param manifest MvFpgManifest
---@return boolean transparent_zero
function malvado.manifest_transparent_zero(manifest) end

---Load a legacy FPG sheet from a directory or sheet image.
---@param path string
---@param kind? "directory"|"image"
---@param rows? integer Required for "image".
---@param columns? integer Required for "image".
---@return userdata legacy_sheet
function malvado.legacy_fpg(path, kind, rows, columns) end

---Create the built-in font descriptor with a default size.
---
---```lua
---local f = font {size = 50}
---```
---@param options MvFontOptions
---@return MvFont font
function malvado.font(options) end

---Load a TTF/OTF font file. Requires an open window.
---
---```lua
---local f = load_font {path = "arcade.ttf", size = 24, extra_glyphs = "ΩЖ"}
---```
---@param options MvLoadFontOptions
---@return MvFont font
function malvado.load_font(options) end

---Create persistent text owned by the current process.
---
---The text stays visible until it is deleted with `delete_text` or until its
---owning process terminates or is killed. Returns a numeric text ID.
---Per-text `color` and `size` are independent of other texts sharing the font.
---
---```lua
---local title = write {
---  font = f,
---  text = "Welcome 2 Malvado!",
---  x = 400, y = 300,
---  align = "center",
---  color = {255, 255, 255, 255},
---  size = 50,
---}
---```
---@param options MvWriteOptions
---@return integer id
function malvado.write(options) end

---Change a persistent text partially. Only provided fields change.
---The text ID and its process owner stay the same. Invalid input changes nothing.
---
---```lua
---change_text(id, {text = "New message!", color = {255, 0, 0, 180}})
---```
---@param id integer
---@param options MvChangeTextOptions
function malvado.change_text(id, options) end

---Delete a persistent text immediately. Also removed automatically when its
---owning process terminates or is killed.
---@param id integer
function malvado.delete_text(id) end

---Measure text in pixels without drawing it.
---
---Measure arbitrary text with a font: `text_size(f, "Hello!")`.
---Measure a persistent text with its effective style: `text_size(id)`.
---Returns width and height. Empty text measures (0, 0). The headless backend
---cannot measure and returns (0, 0).
---@overload fun(font: MvFont, text: string):integer, integer
---@overload fun(id: integer):integer, integer
function malvado.text_size(font, text) end

---Queue immediate text until the process next runs.
---No persistent text object is created. Must be called inside a process.
---
---```lua
---draw_text {text = "Hello!", x = 40, y = 40, size = 24}
---```
---@param options MvDrawTextOptions
function malvado.draw_text(options) end

---Set the shared default color for `draw_rect`, `draw_circle`, and `draw_line`.
---@param color MvColor
function malvado.set_color(color) end

---Set the background color. The background is always opaque; the color's
---alpha channel is ignored.
---@param color MvColor
function malvado.clear_screen(color) end

---Queue a fade to a color over a number of ticks. Default 16 ticks.
---
---```lua
---fade_to {color = {0, 0, 0, 255}, ticks = 30}
---```
---@param options {color: MvColor, ticks?: integer}
function malvado.fade_to(options) end

---Start a fade to opaque black.
---@param ticks? integer Default 16.
function malvado.fade_off(ticks) end

---Start a fade to fully transparent.
---@param ticks? integer Default 16.
function malvado.fade_on(ticks) end

---Check whether a fade is running.
---@return boolean active
function malvado.fade_active() end

---Read the current fade color.
---@return MvColorRGBA color
function malvado.fade_color() end

---Queue a filled or outlined rectangle until the process next runs.
---@param options MvDrawRectOptions
function malvado.draw_rect(options) end

---Queue a filled or outlined circle until the process next runs.
---@param options MvDrawCircleOptions
function malvado.draw_circle(options) end

---Queue a line until the process next runs.
---@param options MvDrawLineOptions
function malvado.draw_line(options) end

---Set the window title.
---@param title string
function malvado.set_title(title) end

---Request a window size during a running game. In headless mode, update the virtual size.
---@param width integer Positive logical width in C int range.
---@param height integer Positive logical height in C int range.
function malvado.set_window_size(width, height) end

---Read the current size in logical pixels once startup begins. In headless mode, return the virtual size.
---@return integer width
---@return integer height
function malvado.window_size() end

---Read the monotonic clock in seconds for elapsed wall time.
---@return number seconds
function malvado.time_seconds() end

---Return a pseudorandom integer in [min, max] from Lua's shared generator.
---@param min integer
---@param max integer
---@return integer value
function malvado.rand(min, max) end

---Reseed the generator shared by `rand` and `math.random`.
---With no arguments, generate a seed automatically. Both forms return no values.
---@overload fun()
---@param seed integer Seed for a repeatable sequence.
function malvado.rand_seed(seed) end

---Check whether a key is currently down.
---@param name MvKey
---@return boolean down
function malvado.key(name) end

---Check whether a key was pressed in the current tick.
---@param name MvKey
---@return boolean pressed
function malvado.key_pressed(name) end

---Check whether a key was released in the current tick.
---@param name MvKey
---@return boolean released
function malvado.key_released(name) end

---Read the mouse position in pixels.
---@return integer x
---@return integer y
function malvado.mouse_position() end

---Check whether a mouse button is currently down.
---@param button MvMouseButton
---@return boolean down
function malvado.mouse_button(button) end

---Check whether the button was pressed in the current tick. Reads are non-consuming.
---@param button MvMouseButton
---@return boolean pressed
function malvado.mouse_pressed(button) end

---Check whether the button was released in the current tick. Reads are non-consuming.
---@param button MvMouseButton
---@return boolean released
function malvado.mouse_released(button) end

---Check whether a controller is connected in a slot (0..3).
---@param slot integer
---@return boolean connected
function malvado.pad_connected(slot) end

---Check whether a controller button is down.
---@param slot integer
---@param name MvPadButton
---@return boolean down
function malvado.pad_button(slot, name) end

---Check whether the button was pressed in the current tick. Reads are non-consuming.
---@param slot integer
---@param name MvPadButton
---@return boolean pressed
function malvado.pad_pressed(slot, name) end

---Check whether the button was released in the current tick. Reads are non-consuming.
---@param slot integer
---@param name MvPadButton
---@return boolean released
function malvado.pad_released(slot, name) end

---Read a controller axis value.
---@param slot integer
---@param name MvPadAxis
---@return number value
function malvado.pad_axis(slot, name) end

---Consume one typed character from the shared queue. Returns nil when the queue is empty.
---@return string|nil character
function malvado.text_input_next() end

---Show or hide the on-screen keyboard (Android).
---@param enabled boolean
---@return boolean toggled
function malvado.text_input_set_keyboard(enabled) end

---Load a reusable WAV sample. Opens the audio device when needed.
---@param path string
---@return MvSound sound
function malvado.load_sound(path) end

---Start an independent voice. Returns nil when all voice slots are in use.
---@param sound MvSound
---@param options? MvSoundPlayOptions
---@return MvVoice|nil voice
function malvado.play_sound(sound, options) end

---Stop a voice and release its playback resources. Safe to repeat.
---@param voice MvVoice
function malvado.stop_sound(voice) end

---Pause a voice. Raises an error after the voice has finished.
---@param voice MvVoice
function malvado.pause_sound(voice) end

---Resume a paused voice. Raises an error after the voice has finished.
---@param voice MvVoice
function malvado.resume_sound(voice) end

---Check whether a voice is playing. Returns false while paused or after it finishes.
---@param voice MvVoice
---@return boolean playing
function malvado.sound_playing(voice) end

---Set the voice volume (0..1). Raises an error after the voice has finished.
---@param voice MvVoice
---@param volume number
function malvado.sound_volume(voice, volume) end

---Load a music stream.
---@param path string
---@param options? MvMusicLoadOptions
---@return MvMusic music
function malvado.load_music(path, options) end

---Check whether music is playing.
---@param music MvMusic
---@return boolean playing
function malvado.music_playing(music) end

---Set the music volume (0..1).
---@param music MvMusic
---@param volume number
function malvado.music_volume(music, volume) end

---Play a music stream.
---@param music MvMusic
function malvado.play_music(music) end

---Advance the music stream (manual mode).
---@param music MvMusic
function malvado.update_music(music) end

---Stop a music stream.
---@param music MvMusic
function malvado.stop_music(music) end

---Pause a music stream.
---@param music MvMusic
function malvado.pause_music(music) end

---Resume a paused music stream.
---@param music MvMusic
function malvado.resume_music(music) end

---Install every module function as a global. Optional convenience for
---DIV-style code. Errors when a global already holds a different value.
---
---```lua
---require("malvado").import_globals()
---write { ... }
---```
function malvado.import_globals() end

malvado.KEY_A = "a"
malvado.KEY_B = "b"
malvado.KEY_C = "c"
malvado.KEY_D = "d"
malvado.KEY_E = "e"
malvado.KEY_F = "f"
malvado.KEY_G = "g"
malvado.KEY_H = "h"
malvado.KEY_I = "i"
malvado.KEY_J = "j"
malvado.KEY_K = "k"
malvado.KEY_L = "l"
malvado.KEY_M = "m"
malvado.KEY_N = "n"
malvado.KEY_O = "o"
malvado.KEY_P = "p"
malvado.KEY_Q = "q"
malvado.KEY_R = "r"
malvado.KEY_S = "s"
malvado.KEY_T = "t"
malvado.KEY_U = "u"
malvado.KEY_V = "v"
malvado.KEY_W = "w"
malvado.KEY_X = "x"
malvado.KEY_Y = "y"
malvado.KEY_Z = "z"
malvado.KEY_0 = "0"
malvado.KEY_1 = "1"
malvado.KEY_2 = "2"
malvado.KEY_3 = "3"
malvado.KEY_4 = "4"
malvado.KEY_5 = "5"
malvado.KEY_6 = "6"
malvado.KEY_7 = "7"
malvado.KEY_8 = "8"
malvado.KEY_9 = "9"
malvado.KEY_ESCAPE = "escape"
malvado.KEY_SPACE = "space"
malvado.KEY_LEFT = "left"
malvado.KEY_RIGHT = "right"
malvado.KEY_UP = "up"
malvado.KEY_DOWN = "down"
malvado.KEY_ENTER = "enter"
malvado.KEY_TAB = "tab"
malvado.KEY_BACKSPACE = "backspace"
malvado.MOUSE_LEFT = "left"
malvado.MOUSE_MIDDLE = "middle"
malvado.MOUSE_RIGHT = "right"
malvado.PAD_A = "a"
malvado.PAD_B = "b"
malvado.PAD_X = "x"
malvado.PAD_Y = "y"
malvado.PAD_UP = "up"
malvado.PAD_DOWN = "down"
malvado.PAD_LEFT = "left"
malvado.PAD_RIGHT = "right"
malvado.PAD_LB = "lb"
malvado.PAD_RB = "rb"
malvado.PAD_BACK = "back"
malvado.PAD_START = "start"
malvado.PAD_LEFT_X = "left_x"
malvado.PAD_LEFT_Y = "left_y"
malvado.PAD_RIGHT_X = "right_x"
malvado.PAD_RIGHT_Y = "right_y"
malvado.PAD_LEFT_TRIGGER = "left_trigger"
malvado.PAD_RIGHT_TRIGGER = "right_trigger"
malvado.SIGNAL_SLEEP = "sleep"
malvado.SIGNAL_FREEZE = "freeze"
malvado.SIGNAL_WAKEUP = "wakeup"
malvado.SIGNAL_KILL = "kill"
malvado.STATUS_RUNNING = "running"
malvado.STATUS_SLEEPING = "sleeping"
malvado.STATUS_FROZEN = "frozen"
malvado.STATUS_DEAD = "dead"

return malvado
