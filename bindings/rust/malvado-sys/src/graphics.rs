// Malvado 2; SPDX-License-Identifier: Apache-2.0
//! Unsafe owner-thread graphics operations from the installed SDK.
pub use crate::MvColor;
use std::ffi::{c_char, c_int};
unsafe extern "C" {
    pub fn mv_window_open(width: c_int, height: c_int, title: *const c_char) -> c_int;
    pub fn mv_window_set_title(title: *const c_char) -> c_int;
    pub fn mv_window_size(width: *mut c_int, height: *mut c_int) -> c_int;
    pub fn mv_window_close() -> ();
    pub fn mv_window_should_close() -> c_int;
    pub fn mv_window_begin(background: MvColor) -> ();
    pub fn mv_window_end() -> ();
}
use std::mem::{align_of, offset_of, size_of};
const _: () = {
    let native: &[usize] = &include!(concat!(env!("OUT_DIR"), "/graphics-layout.rs"));
    let rust = [
        size_of::<MvColor>(),
        align_of::<MvColor>(),
        offset_of!(MvColor, r),
        offset_of!(MvColor, g),
        offset_of!(MvColor, b),
        offset_of!(MvColor, a),
        size_of::<c_int>(),
        align_of::<c_int>(),
        MV_KEY_0 as usize,
        MV_KEY_1 as usize,
        MV_KEY_2 as usize,
        MV_KEY_3 as usize,
        MV_KEY_4 as usize,
        MV_KEY_5 as usize,
        MV_KEY_6 as usize,
        MV_KEY_7 as usize,
        MV_KEY_8 as usize,
        MV_KEY_9 as usize,
        MV_KEY_A as usize,
        MV_KEY_B as usize,
        MV_KEY_C as usize,
        MV_KEY_D as usize,
        MV_KEY_E as usize,
        MV_KEY_F as usize,
        MV_KEY_G as usize,
        MV_KEY_H as usize,
        MV_KEY_I as usize,
        MV_KEY_J as usize,
        MV_KEY_K as usize,
        MV_KEY_L as usize,
        MV_KEY_M as usize,
        MV_KEY_N as usize,
        MV_KEY_O as usize,
        MV_KEY_P as usize,
        MV_KEY_Q as usize,
        MV_KEY_R as usize,
        MV_KEY_S as usize,
        MV_KEY_T as usize,
        MV_KEY_U as usize,
        MV_KEY_V as usize,
        MV_KEY_W as usize,
        MV_KEY_X as usize,
        MV_KEY_Y as usize,
        MV_KEY_Z as usize,
        MV_KEY_ESCAPE as usize,
        MV_KEY_SPACE as usize,
        MV_KEY_LEFT as usize,
        MV_KEY_RIGHT as usize,
        MV_KEY_UP as usize,
        MV_KEY_DOWN as usize,
        MV_KEY_ENTER as usize,
        MV_KEY_TAB as usize,
        MV_KEY_BACKSPACE as usize,
        size_of::<MvImageRegion>(),
        align_of::<MvImageRegion>(),
        offset_of!(MvImageRegion, x),
        offset_of!(MvImageRegion, y),
        offset_of!(MvImageRegion, width),
        offset_of!(MvImageRegion, height),
        size_of::<MvImageTransform>(),
        align_of::<MvImageTransform>(),
        offset_of!(MvImageTransform, x),
        offset_of!(MvImageTransform, y),
        offset_of!(MvImageTransform, pivot_x),
        offset_of!(MvImageTransform, pivot_y),
        offset_of!(MvImageTransform, scale),
        offset_of!(MvImageTransform, rotation),
        size_of::<MvMouseButton>(),
        align_of::<MvMouseButton>(),
        MV_MOUSE_LEFT as usize,
        MV_MOUSE_MIDDLE as usize,
        MV_MOUSE_RIGHT as usize,
        size_of::<MvPadButton>(),
        align_of::<MvPadButton>(),
        MV_PAD_A as usize,
        MV_PAD_B as usize,
        MV_PAD_X as usize,
        MV_PAD_Y as usize,
        MV_PAD_UP as usize,
        MV_PAD_DOWN as usize,
        MV_PAD_LEFT as usize,
        MV_PAD_RIGHT as usize,
        MV_PAD_LB as usize,
        MV_PAD_RB as usize,
        MV_PAD_BACK as usize,
        MV_PAD_START as usize,
        size_of::<MvPadAxis>(),
        align_of::<MvPadAxis>(),
        MV_PAD_LEFT_X as usize,
        MV_PAD_LEFT_Y as usize,
        MV_PAD_RIGHT_X as usize,
        MV_PAD_RIGHT_Y as usize,
        MV_PAD_LEFT_TRIGGER as usize,
        MV_PAD_RIGHT_TRIGGER as usize,
        size_of::<MvFade>(),
        align_of::<MvFade>(),
        offset_of!(MvFade, current),
        offset_of!(MvFade, start),
        offset_of!(MvFade, target),
        offset_of!(MvFade, duration),
        offset_of!(MvFade, elapsed),
        size_of::<MvFadeTransition>(),
        align_of::<MvFadeTransition>(),
        offset_of!(MvFadeTransition, target),
        offset_of!(MvFadeTransition, ticks),
        size_of::<MvFadeQueue>(),
        align_of::<MvFadeQueue>(),
        offset_of!(MvFadeQueue, fade),
        offset_of!(MvFadeQueue, pending),
        offset_of!(MvFadeQueue, head),
        offset_of!(MvFadeQueue, count),
        size_of::<MvDrawState>(),
        align_of::<MvDrawState>(),
        offset_of!(MvDrawState, color),
        offset_of!(MvDrawState, background),
        size_of::<MvImageGraphic>(),
        align_of::<MvImageGraphic>(),
        offset_of!(MvImageGraphic, image),
        offset_of!(MvImageGraphic, region),
        offset_of!(MvImageGraphic, pivot_x),
        offset_of!(MvImageGraphic, pivot_y),
    ];
    assert!(native.len() == rust.len(), "graphics ABI count mismatch");
    let mut i = 0;
    while i < rust.len() {
        assert!(native[i] == rust[i], "graphics ABI layout/value mismatch");
        i += 1;
    }
};

pub const MV_KEY_0: c_int = 48;
pub const MV_KEY_1: c_int = 49;
pub const MV_KEY_2: c_int = 50;
pub const MV_KEY_3: c_int = 51;
pub const MV_KEY_4: c_int = 52;
pub const MV_KEY_5: c_int = 53;
pub const MV_KEY_6: c_int = 54;
pub const MV_KEY_7: c_int = 55;
pub const MV_KEY_8: c_int = 56;
pub const MV_KEY_9: c_int = 57;
pub const MV_KEY_A: c_int = 65;
pub const MV_KEY_B: c_int = 66;
pub const MV_KEY_C: c_int = 67;
pub const MV_KEY_D: c_int = 68;
pub const MV_KEY_E: c_int = 69;
pub const MV_KEY_F: c_int = 70;
pub const MV_KEY_G: c_int = 71;
pub const MV_KEY_H: c_int = 72;
pub const MV_KEY_I: c_int = 73;
pub const MV_KEY_J: c_int = 74;
pub const MV_KEY_K: c_int = 75;
pub const MV_KEY_L: c_int = 76;
pub const MV_KEY_M: c_int = 77;
pub const MV_KEY_N: c_int = 78;
pub const MV_KEY_O: c_int = 79;
pub const MV_KEY_P: c_int = 80;
pub const MV_KEY_Q: c_int = 81;
pub const MV_KEY_R: c_int = 82;
pub const MV_KEY_S: c_int = 83;
pub const MV_KEY_T: c_int = 84;
pub const MV_KEY_U: c_int = 85;
pub const MV_KEY_V: c_int = 86;
pub const MV_KEY_W: c_int = 87;
pub const MV_KEY_X: c_int = 88;
pub const MV_KEY_Y: c_int = 89;
pub const MV_KEY_Z: c_int = 90;
pub const MV_KEY_ESCAPE: c_int = 256;
pub const MV_KEY_SPACE: c_int = 257;
pub const MV_KEY_LEFT: c_int = 258;
pub const MV_KEY_RIGHT: c_int = 259;
pub const MV_KEY_UP: c_int = 260;
pub const MV_KEY_DOWN: c_int = 261;
pub const MV_KEY_ENTER: c_int = 262;
pub const MV_KEY_TAB: c_int = 263;
pub const MV_KEY_BACKSPACE: c_int = 264;
unsafe extern "C" {
    pub fn mv_input_tick();
    pub fn mv_key_down(key: c_int) -> c_int;
    pub fn mv_key_pressed(key: c_int) -> c_int;
    pub fn mv_key_released(key: c_int) -> c_int;
}

unsafe extern "C" {
    pub fn mv_window_clip_begin(x: c_int, y: c_int, width: c_int, height: c_int) -> c_int;
    pub fn mv_window_clip_end() -> c_int;
    pub fn mv_window_scroll_begin(x: f32, y: f32) -> c_int;
    pub fn mv_window_scroll_end() -> c_int;
    pub fn mv_window_draw_text(
        text: *const c_char,
        x: c_int,
        y: c_int,
        size: c_int,
        color: MvColor,
    ) -> ();
    pub fn mv_window_text_width(text: *const c_char, size: c_int) -> c_int;
    pub fn mv_window_draw_box(
        x: c_int,
        y: c_int,
        width: c_int,
        height: c_int,
        filled: c_int,
        color: MvColor,
    ) -> c_int;
    pub fn mv_window_draw_circle(
        x: c_int,
        y: c_int,
        radius: c_int,
        filled: c_int,
        color: MvColor,
    ) -> c_int;
    pub fn mv_window_draw_transformed_rect(
        x: f32,
        y: f32,
        width: f32,
        height: f32,
        pivot_x: f32,
        pivot_y: f32,
        angle: f32,
        color: MvColor,
    ) -> c_int;
    pub fn mv_window_draw_filled_circle(x: f32, y: f32, radius: f32, color: MvColor) -> c_int;
}

#[repr(C)]
pub struct MvImage {
    _private: [u8; 0],
}
#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub struct MvImageRegion {
    pub x: c_int,
    pub y: c_int,
    pub width: c_int,
    pub height: c_int,
}
#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub struct MvImageTransform {
    pub x: f32,
    pub y: f32,
    pub pivot_x: f32,
    pub pivot_y: f32,
    pub scale: f32,
    pub rotation: f32,
}
unsafe extern "C" {
    pub fn mv_image_load(path: *const c_char) -> *mut MvImage;
    pub fn mv_image_from_png(data: *const u8, length: usize) -> *mut MvImage;
    pub fn mv_image_from_rgba(
        width: c_int,
        height: c_int,
        pixels: *const u8,
        length: usize,
    ) -> *mut MvImage;
    pub fn mv_image_retain(image: *mut MvImage) -> c_int;
    pub fn mv_image_destroy(image: *mut MvImage) -> ();
    pub fn mv_image_width(image: *const MvImage) -> c_int;
    pub fn mv_image_height(image: *const MvImage) -> c_int;
    pub fn mv_image_draw(image: *const MvImage, x: c_int, y: c_int) -> ();
    pub fn mv_image_region_valid(
        image: *const MvImage,
        region: MvImageRegion,
        transform: MvImageTransform,
    ) -> c_int;
    pub fn mv_image_draw_region(
        image: *const MvImage,
        region: MvImageRegion,
        transform: MvImageTransform,
    ) -> c_int;
}

#[repr(C)]
pub struct MvFont {
    _private: [u8; 0],
}
unsafe extern "C" {
    pub fn mv_font_load(path: *const c_char, base_size: c_int) -> *mut MvFont;
    pub fn mv_font_load_codepoints(
        path: *const c_char,
        base_size: c_int,
        codepoints: *const u32,
        count: usize,
    ) -> *mut MvFont;
    pub fn mv_font_destroy(font: *mut MvFont) -> ();
    pub fn mv_font_text_width(font: *const MvFont, text: *const c_char, size: c_int) -> c_int;
    pub fn mv_font_draw(
        font: *const MvFont,
        text: *const c_char,
        x: c_int,
        y: c_int,
        size: c_int,
        color: MvColor,
    ) -> c_int;
}

unsafe extern "C" {
    pub fn mv_mouse_position(x: *mut c_int, y: *mut c_int) -> c_int;
    pub fn mv_mouse_down(button: MvMouseButton) -> c_int;
    pub fn mv_pad_connected(slot: c_int) -> c_int;
    pub fn mv_pad_button_down(slot: c_int, button: MvPadButton) -> c_int;
    pub fn mv_pad_axis(slot: c_int, axis: MvPadAxis) -> f32;
    pub fn mv_text_input_next(codepoint: *mut u32) -> c_int;
}

pub type MvMouseButton = c_int;
pub const MV_MOUSE_LEFT: c_int = 1;
pub const MV_MOUSE_MIDDLE: c_int = 2;
pub const MV_MOUSE_RIGHT: c_int = 3;
pub type MvPadButton = c_int;
pub const MV_PAD_A: c_int = 0;
pub const MV_PAD_B: c_int = 1;
pub const MV_PAD_X: c_int = 2;
pub const MV_PAD_Y: c_int = 3;
pub const MV_PAD_UP: c_int = 4;
pub const MV_PAD_DOWN: c_int = 5;
pub const MV_PAD_LEFT: c_int = 6;
pub const MV_PAD_RIGHT: c_int = 7;
pub const MV_PAD_LB: c_int = 8;
pub const MV_PAD_RB: c_int = 9;
pub const MV_PAD_BACK: c_int = 10;
pub const MV_PAD_START: c_int = 11;
pub type MvPadAxis = c_int;
pub const MV_PAD_LEFT_X: c_int = 0;
pub const MV_PAD_LEFT_Y: c_int = 1;
pub const MV_PAD_RIGHT_X: c_int = 2;
pub const MV_PAD_RIGHT_Y: c_int = 3;
pub const MV_PAD_LEFT_TRIGGER: c_int = 4;
pub const MV_PAD_RIGHT_TRIGGER: c_int = 5;

#[repr(C)]
pub struct MvSound {
    _private: [u8; 0],
}
unsafe extern "C" {
    pub fn mv_audio_open() -> c_int;
    pub fn mv_audio_close();
    pub fn mv_sound_load(path: *const c_char) -> *mut MvSound;
    pub fn mv_sound_clone(sound: *const MvSound) -> *mut MvSound;
    pub fn mv_sound_destroy(sound: *mut MvSound);
    pub fn mv_sound_play(sound: *mut MvSound) -> c_int;
    pub fn mv_sound_stop(sound: *mut MvSound) -> c_int;
    pub fn mv_sound_pause(sound: *mut MvSound) -> c_int;
    pub fn mv_sound_resume(sound: *mut MvSound) -> c_int;
    pub fn mv_sound_playing(sound: *const MvSound) -> c_int;
    pub fn mv_sound_set_volume(sound: *mut MvSound, volume: f32) -> c_int;
}

#[repr(C)]
pub struct MvMusic {
    _private: [u8; 0],
}
unsafe extern "C" {
    pub fn mv_music_load(path: *const c_char, looping: c_int) -> *mut MvMusic;
    pub fn mv_music_destroy(music: *mut MvMusic);
    pub fn mv_music_play(music: *mut MvMusic) -> c_int;
    pub fn mv_music_update(music: *mut MvMusic) -> c_int;
    pub fn mv_music_stop(music: *mut MvMusic) -> c_int;
    pub fn mv_music_pause(music: *mut MvMusic) -> c_int;
    pub fn mv_music_resume(music: *mut MvMusic) -> c_int;
    pub fn mv_music_playing(music: *const MvMusic) -> c_int;
    pub fn mv_music_set_volume(music: *mut MvMusic, volume: f32) -> c_int;
}

#[repr(C)]
#[derive(Clone, Copy, Debug, Default)]
pub struct MvFade {
    pub current: MvColor,
    pub start: MvColor,
    pub target: MvColor,
    pub duration: u32,
    pub elapsed: u32,
}
#[repr(C)]
#[derive(Clone, Copy, Debug, Default)]
pub struct MvFadeTransition {
    pub target: MvColor,
    pub ticks: u32,
}
#[repr(C)]
#[derive(Clone, Copy, Debug, Default)]
pub struct MvFadeQueue {
    pub fade: MvFade,
    pub pending: [MvFadeTransition; 16],
    pub head: u32,
    pub count: u32,
}
unsafe extern "C" {
    pub fn mv_fade_queue_push(queue: *mut MvFadeQueue, target: MvColor, ticks: u32) -> c_int;
    pub fn mv_fade_queue_step(queue: *mut MvFadeQueue) -> c_int;
    pub fn mv_fade_queue_active(queue: *const MvFadeQueue) -> c_int;
    pub fn mv_window_draw_fade(color: MvColor) -> c_int;
}

unsafe extern "C" {
    pub fn mv_window_draw_process_graphics(ctx: *mut crate::MvContext) -> c_int;
    pub fn mv_window_draw_graphic(
        shape: crate::MvCollisionShape,
        transform: crate::MvCollisionTransform,
        color: MvColor,
    ) -> c_int;
}

#[repr(C)]
#[derive(Clone, Copy, Debug, Default)]
pub struct MvDrawState {
    pub color: MvColor,
    pub background: MvColor,
}
unsafe extern "C" {
    pub fn mv_draw_state_init(state: *mut MvDrawState) -> c_int;
    pub fn mv_draw_state_color(state: *mut MvDrawState, r: i64, g: i64, b: i64, a: i64) -> c_int;
    pub fn mv_draw_state_background(state: *mut MvDrawState, r: i64, g: i64, b: i64) -> c_int;
}

unsafe extern "C" {
    pub fn mv_resource_adopt_image(
        ctx: *mut crate::MvContext,
        owner: *const crate::MvProcessRef,
        data: *mut MvImage,
        out: *mut *mut crate::MvResource,
    ) -> c_int;
    pub fn mv_resource_adopt_font(
        ctx: *mut crate::MvContext,
        owner: *const crate::MvProcessRef,
        data: *mut MvFont,
        out: *mut *mut crate::MvResource,
    ) -> c_int;
    pub fn mv_resource_adopt_sound(
        ctx: *mut crate::MvContext,
        owner: *const crate::MvProcessRef,
        data: *mut MvSound,
        out: *mut *mut crate::MvResource,
    ) -> c_int;
    pub fn mv_resource_adopt_music(
        ctx: *mut crate::MvContext,
        owner: *const crate::MvProcessRef,
        data: *mut MvMusic,
        out: *mut *mut crate::MvResource,
    ) -> c_int;
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct MvImageGraphic {
    pub image: *mut MvImage,
    pub region: MvImageRegion,
    pub pivot_x: f64,
    pub pivot_y: f64,
}
unsafe extern "C" {
    pub fn mv_process_image_set(
        ctx: *mut crate::MvContext,
        process: crate::MvProcessRef,
        graphic: MvImageGraphic,
    ) -> c_int;
    pub fn mv_process_image_set_owned(
        ctx: *mut crate::MvContext,
        process: crate::MvProcessRef,
        graphic: MvImageGraphic,
        owner: *mut std::ffi::c_void,
        release: Option<unsafe extern "C" fn(*mut std::ffi::c_void)>,
    ) -> c_int;
    pub fn mv_process_image_get(
        ctx: *mut crate::MvContext,
        process: crate::MvProcessRef,
        out: *mut MvImageGraphic,
        present: *mut c_int,
    ) -> c_int;
}
