-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0
--
-- Global definitions for the Lua Language Server (LuaLS). Opt-in.
--
-- Enable this file only for projects that call
-- `require("malvado").import_globals()` in their entry point. Projects that
-- use the namespaced API (`local mv = require("malvado")`) do not need these
-- globals: remove this file (or the library entry) from .luarc.json.
--
-- This file only declares the globals installed by import_globals(); it does
-- not execute runtime functionality. See malvado.d.lua for the full module.

---@meta

---Define a process. Calling this table defines a process and returns its
---constructor; calling the constructor spawns the process and returns its
---handle. The table also exposes `process.spawn`, `process.find`,
---`process.kill`, the other process operations and the SIGNAL_* constants.
---@type MvProcessNamespace
process = {}

---@param constructor MvProcessConstructor
---@param fields? table
---@param options? MvSpawnOptions
---@return MvProcess handle
function spawn(constructor, fields, options) end

---@param constructor MvProcessConstructor
---@return MvProcess[] handles
function find(constructor) end

---@param handle MvProcess
---@param options? MvTreeOptions
---@return MvProcess[] children
function children(handle, options) end

---@param handle MvProcess
---@param options? MvTreeOptions
---@return MvProcess[] descendants
function descendants(handle, options) end

---@return MvSnapshot snapshot
function snapshot() end

function frame() end

---@param handle MvProcess
---@param signal MvSignal
function signal(handle, signal) end

---@param handle MvProcess
---@param signal MvSignal
---@param options? MvTreeOptions
function signal_tree(handle, signal, options) end

---@param handle MvProcess
---@param priority integer
function set_priority(handle, priority) end

---@param handle MvProcess
---@return integer priority
function get_priority(handle) end

---@param handle MvProcess
function kill(handle) end

---@param handle MvProcess
---@return boolean alive
function alive(handle) end

---@param handle MvProcess
---@param options? MvTreeOptions
function kill_tree(handle, options) end

---@param handle MvProcess
---@return MvProcessStatusName|nil status
function status(handle) end

---@param handle MvProcess
---@param message any
function send(handle, message) end

---@param body fun()
---@param options? MvStartOptions
function start(body, options) end

---@param options MvRectOptions
---@return MvGraphic
function rect(options) end

---@param options MvCircleOptions
---@return MvGraphic
function circle(options) end

---@param x integer
---@param y integer
---@param width integer
---@param height integer
---@return MvRegion
function region(x, y, width, height) end

---@param x number
---@param y number
---@return MvScroll
function scroll(x, y) end

---@param ax number
---@param ay number
---@param aw number
---@param ah number
---@param bx number
---@param by number
---@param bw number
---@param bh number
---@return boolean overlaps
function rect_overlaps(ax, ay, aw, ah, bx, by, bw, bh) end

---@param rx number
---@param ry number
---@param rw number
---@param rh number
---@param px number
---@param py number
---@return boolean contains
function rect_contains(rx, ry, rw, rh, px, py) end

---@param x number
---@param y number
---@param options? MvCameraCreateOptions
---@return MvCamera camera
function camera_create(x, y, options) end

---@overload fun(x: number, y: number)
---@param camera MvCamera
---@param x number
---@param y number
function camera_set(camera, x, y) end

---@overload fun(dx: number, dy: number)
---@param camera MvCamera
---@param dx number
---@param dy number
function camera_move(camera, dx, dy) end

---@param target MvProcess
function camera_follow(target) end

---@param camera? MvCamera
---@return number x
---@return number y
function camera_position(camera) end

---@return MvCollisionWorld world
function world_create() end

---@param dx number
---@param dy number
---@return MvMoveResult result
function world_move(dx, dy) end

---@overload fun(path: string, options: MvLoadAssetPivotOptions):MvImage
---@overload fun(path: string, options: {format: "png"}):MvImage
---@overload fun(path: string, options: {format: "fpg"}):MvFpg
---@overload fun(path: string, options: {format: "fpg-manifest"}):MvFpgManifest
---@overload fun(path: string, options: {format: "atlas"|"aseprite"}):MvAtlas
---@param path string
---@param options? MvLoadAssetOptions
---@return MvImage|MvAtlas|MvFpg|MvFpgManifest
function load_asset(path, options) end

---@param path string
---@return MvImage image
function load_image(path) end

---@param image MvImage
---@return integer width
---@return integer height
function image_size(image) end

---Queue an image draw until the process next activates.
---Must be called inside a process.
---@param options MvDrawImageOptions
function draw_image(options) end

---@param image MvImage
---@param json string
---@return MvAtlas atlas
function load_atlas(image, json) end

---@param image MvImage
---@param json string
---@return MvAtlas atlas
function load_aseprite(image, json) end

---@param atlas MvAtlas
---@param id integer
---@return MvSpriteInfo info
function sprite_info(atlas, id) end

---@param atlas MvAtlas
---@param name string
---@return integer|nil id
function sprite_id(atlas, name) end

---@param atlas MvAtlas
---@param name string
---@param elapsed_ms integer
---@param loop? boolean Default true.
---@return integer sprite_id
function animation_frame(atlas, name, elapsed_ms, loop) end

---Read elapsed playback in milliseconds. Returns nil for a static graph.
---@param handle MvProcess
---@return number|nil elapsed_ms
function animation_elapsed(handle) end

---@param atlas MvAtlas
---@param name string
---@return integer[]|nil color
function animation_color(atlas, name) end

---@param atlas MvAtlas
---@return integer[]|nil color
function atlas_pivot_color(atlas) end

---@param atlas MvAtlas
---@param name string
---@return integer|nil repeats
function animation_repeats(atlas, name) end

---Queue an atlas sprite draw with its metadata pivot until the process next activates.
---Must be called inside a process.
---@param options MvDrawSpriteOptions
function draw_sprite(options) end

---@overload fun(library: MvAtlas|MvFpg|MvFpgManifest, id: integer):MvGraphic
---@overload fun(id: integer):MvGraphic
---@return MvGraphic
function graph(library, id) end

---Create an animation descriptor from an atlas animation or a frame list.
---@overload fun(library: MvAtlas, name: string, options?: MvAnimationOptions):MvAnimation
---@overload fun(library: MvAtlas|MvFpg|MvFpgManifest, sequence: integer[], options?: MvAnimationOptions):MvAnimation
---@overload fun(name: string, options?: MvAnimationOptions):MvAnimation
---@overload fun(sequence: integer[], options?: MvAnimationOptions):MvAnimation
---@return MvAnimation
function animation(sequence, options) end

---@param options MvFontOptions
---@return MvFont font
function font(options) end

---@param options MvLoadFontOptions
---@return MvFont font
function load_font(options) end

---Create persistent text owned by the current process.
---The text stays visible until it is deleted with `delete_text` or until its
---owning process terminates or is killed. Returns a numeric text ID.
---@param options MvWriteOptions
---@return integer id
function write(options) end

---Change a persistent text partially. Only provided fields change.
---@param id integer
---@param options MvChangeTextOptions
function change_text(id, options) end

---Delete a persistent text immediately.
---@param id integer
function delete_text(id) end

---Measure text in pixels without drawing it.
---@overload fun(font: MvFont, text: string):integer, integer
---@overload fun(id: integer):integer, integer
function text_size(font, text) end

---Queue immediate text until the process next runs.
---@param options MvDrawTextOptions
function draw_text(options) end

---@type MvPaletteColor
BLACK = nil

---@type MvPaletteColor
WHITE = nil

---@type MvPaletteColor
TRANSPARENT = nil

---@type MvPaletteColor
GRAY = nil

---@type MvPaletteColor
LIGHT_GRAY = nil

---@type MvPaletteColor
DARK_GRAY = nil

---@type MvPaletteColor
RED = nil

---@type MvPaletteColor
GREEN = nil

---@type MvPaletteColor
BLUE = nil

---@type MvPaletteColor
YELLOW = nil

---@type MvPaletteColor
CYAN = nil

---@type MvPaletteColor
MAGENTA = nil

---@type MvPaletteColor
ORANGE = nil

---@type MvPaletteColor
PURPLE = nil

---@type MvPaletteColor
PINK = nil

---@type MvPaletteColor
BROWN = nil

---@param color MvColor
function set_color(color) end

---@param color MvColor
function clear_screen(color) end

---@param options {color: MvColor, ticks?: integer}
function fade_to(options) end

---@param ticks? integer Default 16.
function fade_off(ticks) end

---@param ticks? integer Default 16.
function fade_on(ticks) end

---@return boolean active
function fade_active() end

---@return MvColorRGBA color
function fade_color() end

---Queue a filled or outlined rectangle until the process next runs.
---@param options MvDrawRectOptions
function draw_rect(options) end

---Queue a filled or outlined circle until the process next runs.
---@param options MvDrawCircleOptions
function draw_circle(options) end

---Queue a line until the process next runs.
---@param options MvDrawLineOptions
function draw_line(options) end

---@param title string
function set_title(title) end

---@return integer width
---@return integer height
function window_size() end

---@return number seconds
function time_seconds() end

---@return integer width
function get_screen_width() end

---@return integer height
function get_screen_height() end

---@param min integer
---@param max integer
---@return integer value
function rand(min, max) end

---@param name MvKey
---@return boolean down
function key(name) end

---@param name MvKey
---@return boolean down
function key_down(name) end

---@param name MvKey
---@return boolean pressed
function key_pressed(name) end

---@param name MvKey
---@return boolean released
function key_released(name) end

---@return integer x
---@return integer y
function mouse_position() end

---@param button MvMouseButton
---@return boolean down
function mouse_button(button) end

---@param slot integer
---@return boolean connected
function pad_connected(slot) end

---@param slot integer
---@param name string
---@return boolean down
function pad_button(slot, name) end

---@param slot integer
---@param name string
---@return number value
function pad_axis(slot, name) end

---@return string|nil character
function text_input_next() end

---@param enabled boolean
---@return boolean toggled
function text_input_set_keyboard(enabled) end

---@param path string
---@return MvSound sound
function load_sound(path) end

---@param sound MvSound
---@return MvSound clone
function clone_sound(sound) end

---@param sound MvSound
function play_sound(sound) end

---@param sound MvSound
function stop_sound(sound) end

---@param sound MvSound
function pause_sound(sound) end

---@param sound MvSound
function resume_sound(sound) end

---@param sound MvSound
---@return boolean playing
function sound_playing(sound) end

---@param sound MvSound
---@param volume number
function sound_volume(sound, volume) end

---@param path string
---@param loop? boolean Default false.
---@return MvMusic music
function load_music(path, loop) end

---@param music MvMusic
---@return boolean playing
function music_playing(music) end

---@param music MvMusic
---@param volume number
function music_volume(music, volume) end

---@param music MvMusic
function play_music(music) end

---@param music MvMusic
function update_music(music) end

---@param music MvMusic
function stop_music(music) end

---@param music MvMusic
function pause_music(music) end

---@param music MvMusic
function resume_music(music) end

---Query the first collision of the current process against one process type.
---Must be called inside a process. The table also exposes `collision.circle`,
---`collision.rect` and `collision.overlap`.
---@type MvCollisionNamespace
collision = {}

KEY_A = "a"
KEY_B = "b"
KEY_C = "c"
KEY_D = "d"
KEY_E = "e"
KEY_F = "f"
KEY_G = "g"
KEY_H = "h"
KEY_I = "i"
KEY_J = "j"
KEY_K = "k"
KEY_L = "l"
KEY_M = "m"
KEY_N = "n"
KEY_O = "o"
KEY_P = "p"
KEY_Q = "q"
KEY_R = "r"
KEY_S = "s"
KEY_T = "t"
KEY_U = "u"
KEY_V = "v"
KEY_W = "w"
KEY_X = "x"
KEY_Y = "y"
KEY_Z = "z"
KEY_0 = "0"
KEY_1 = "1"
KEY_2 = "2"
KEY_3 = "3"
KEY_4 = "4"
KEY_5 = "5"
KEY_6 = "6"
KEY_7 = "7"
KEY_8 = "8"
KEY_9 = "9"
KEY_ESCAPE = "escape"
KEY_SPACE = "space"
KEY_LEFT = "left"
KEY_RIGHT = "right"
KEY_UP = "up"
KEY_DOWN = "down"
KEY_ENTER = "enter"
KEY_TAB = "tab"
KEY_BACKSPACE = "backspace"
SIGNAL_SLEEP = "sleep"
SIGNAL_FREEZE = "freeze"
SIGNAL_WAKEUP = "wakeup"
SIGNAL_KILL = "kill"
