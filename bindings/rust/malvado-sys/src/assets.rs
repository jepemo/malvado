// Malvado 2; SPDX-License-Identifier: Apache-2.0
//! Optional native atlas declarations. All borrowed data ends with the atlas.
use std::ffi::{c_char, c_int};
#[repr(C)]
pub struct MvAtlas {
    _private: [u8; 0],
}
#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct MvColor {
    pub r: u8,
    pub g: u8,
    pub b: u8,
    pub a: u8,
}
#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct MvImageRegion {
    pub x: c_int,
    pub y: c_int,
    pub width: c_int,
    pub height: c_int,
}
#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct MvAtlasSprite {
    pub id: c_int,
    pub name: *const c_char,
    pub region: MvImageRegion,
    pub pivot_x: c_int,
    pub pivot_y: c_int,
    pub source_width: c_int,
    pub source_height: c_int,
    pub trim_x: c_int,
    pub trim_y: c_int,
}
#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct MvAtlasAnimationFrame {
    pub sprite_id: c_int,
    pub duration_ms: u32,
}
unsafe extern "C" {
    pub fn mv_atlas_parse(
        json: *const c_char,
        length: usize,
        width: c_int,
        height: c_int,
        error: *mut *const c_char,
    ) -> *mut MvAtlas;
    pub fn mv_atlas_parse_aseprite(
        json: *const c_char,
        length: usize,
        width: c_int,
        height: c_int,
        error: *mut *const c_char,
    ) -> *mut MvAtlas;
    pub fn mv_atlas_destroy(atlas: *mut MvAtlas) -> ();
    pub fn mv_atlas_image(atlas: *const MvAtlas) -> *const c_char;
    pub fn mv_atlas_count(atlas: *const MvAtlas) -> usize;
    pub fn mv_atlas_find(atlas: *const MvAtlas, id: c_int, sprite: *mut MvAtlasSprite) -> c_int;
    pub fn mv_atlas_find_name(
        atlas: *const MvAtlas,
        name: *const c_char,
        sprite: *mut MvAtlasSprite,
    ) -> c_int;
    pub fn mv_atlas_pivot_color(atlas: *const MvAtlas, out: *mut MvColor) -> c_int;
    pub fn mv_atlas_animation_color(
        atlas: *const MvAtlas,
        name: *const c_char,
        out: *mut MvColor,
    ) -> c_int;
    pub fn mv_atlas_animation_repeats(
        atlas: *const MvAtlas,
        name: *const c_char,
        out: *mut u32,
    ) -> c_int;
    pub fn mv_atlas_animation_frame(
        atlas: *const MvAtlas,
        name: *const c_char,
        elapsed_ms: u64,
        r#loop: c_int,
        sprite_id: *mut c_int,
    ) -> c_int;
    pub fn mv_atlas_animation_frame_count(atlas: *const MvAtlas, name: *const c_char) -> usize;
    pub fn mv_atlas_animation_frame_at(
        atlas: *const MvAtlas,
        name: *const c_char,
        index: usize,
        out: *mut MvAtlasAnimationFrame,
    ) -> c_int;
}
use std::mem::{align_of, offset_of, size_of};
const _: () = {
    let native: &[usize] = &include!(concat!(env!("OUT_DIR"), "/assets-layout.rs"));
    let rust = [
        size_of::<MvColor>(),
        align_of::<MvColor>(),
        offset_of!(MvColor, r),
        offset_of!(MvColor, g),
        offset_of!(MvColor, b),
        offset_of!(MvColor, a),
        size_of::<MvImageRegion>(),
        align_of::<MvImageRegion>(),
        offset_of!(MvImageRegion, x),
        offset_of!(MvImageRegion, y),
        offset_of!(MvImageRegion, width),
        offset_of!(MvImageRegion, height),
        size_of::<MvAtlasSprite>(),
        align_of::<MvAtlasSprite>(),
        offset_of!(MvAtlasSprite, id),
        offset_of!(MvAtlasSprite, name),
        offset_of!(MvAtlasSprite, region),
        offset_of!(MvAtlasSprite, pivot_x),
        offset_of!(MvAtlasSprite, pivot_y),
        offset_of!(MvAtlasSprite, source_width),
        offset_of!(MvAtlasSprite, source_height),
        offset_of!(MvAtlasSprite, trim_x),
        offset_of!(MvAtlasSprite, trim_y),
        size_of::<MvAtlasAnimationFrame>(),
        align_of::<MvAtlasAnimationFrame>(),
        offset_of!(MvAtlasAnimationFrame, sprite_id),
        offset_of!(MvAtlasAnimationFrame, duration_ms),
    ];
    assert!(
        native.len() == rust.len(),
        "assets ABI layout count mismatch"
    );
    let mut i = 0;
    while i < rust.len() {
        assert!(native[i] == rust[i], "assets ABI layout/value mismatch");
        i += 1;
    }
};
