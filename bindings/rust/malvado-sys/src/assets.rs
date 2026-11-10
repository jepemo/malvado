// Malvado 2; SPDX-License-Identifier: Apache-2.0
//! Optional native atlas and FPG declarations. All borrowed data ends with its native owner.
use std::ffi::{c_char, c_int, c_void};
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
        size_of::<MvFpgSprite>(),
        align_of::<MvFpgSprite>(),
        offset_of!(MvFpgSprite, id),
        offset_of!(MvFpgSprite, width),
        offset_of!(MvFpgSprite, height),
        offset_of!(MvFpgSprite, name),
        offset_of!(MvFpgSprite, filename),
        offset_of!(MvFpgSprite, point_count),
        offset_of!(MvFpgSprite, pixels),
        size_of::<MvFpgPoint>(),
        align_of::<MvFpgPoint>(),
        offset_of!(MvFpgPoint, x),
        offset_of!(MvFpgPoint, y),
        offset_of!(MvFpgPoint, defined),
        size_of::<MvFpgManifestSprite>(),
        align_of::<MvFpgManifestSprite>(),
        offset_of!(MvFpgManifestSprite, id),
        offset_of!(MvFpgManifestSprite, width),
        offset_of!(MvFpgManifestSprite, height),
        offset_of!(MvFpgManifestSprite, name),
        offset_of!(MvFpgManifestSprite, filename),
        offset_of!(MvFpgManifestSprite, point_count),
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

#[repr(C)]
pub struct MvFpg {
    _private: [u8; 0],
}
#[repr(C)]
pub struct MvFpgManifest {
    _private: [u8; 0],
}
#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct MvFpgSprite {
    pub id: c_int,
    pub width: c_int,
    pub height: c_int,
    pub name: [u8; 32],
    pub filename: [u8; 12],
    pub point_count: usize,
    pub pixels: *const u8,
}
#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct MvFpgPoint {
    pub x: c_int,
    pub y: c_int,
    pub defined: c_int,
}
#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct MvFpgManifestSprite {
    pub id: c_int,
    pub width: c_int,
    pub height: c_int,
    pub name: [u8; 32],
    pub filename: [u8; 12],
    pub point_count: usize,
}
unsafe extern "C" {
    pub fn mv_fpg_parse(
        data: *const c_void,
        length: usize,
        error: *mut *const c_char,
    ) -> *mut MvFpg;
    pub fn mv_fpg_destroy(fpg: *mut MvFpg) -> ();
    pub fn mv_fpg_count(fpg: *const MvFpg) -> usize;
    pub fn mv_fpg_palette(fpg: *const MvFpg) -> *const u8;
    pub fn mv_fpg_gamma(fpg: *const MvFpg) -> *const u8;
    pub fn mv_fpg_sprite(fpg: *const MvFpg, id: c_int, out: *mut MvFpgSprite) -> c_int;
    pub fn mv_fpg_at(fpg: *const MvFpg, index: usize, out: *mut MvFpgSprite) -> c_int;
    pub fn mv_fpg_point(fpg: *const MvFpg, id: c_int, index: usize, out: *mut MvFpgPoint) -> c_int;
    pub fn mv_fpg_copy_rgba(
        fpg: *const MvFpg,
        id: c_int,
        transparent_zero: c_int,
        output: *mut u8,
        length: usize,
    ) -> c_int;
    pub fn mv_fpg_png_size(fpg: *const MvFpg, id: c_int, transparent_zero: c_int) -> usize;
    pub fn mv_fpg_copy_png(
        fpg: *const MvFpg,
        id: c_int,
        transparent_zero: c_int,
        output: *mut u8,
        length: usize,
    ) -> c_int;
    pub fn mv_fpg_manifest_parse(
        json: *const c_char,
        length: usize,
        error: *mut *const c_char,
    ) -> *mut MvFpgManifest;
    pub fn mv_fpg_manifest_destroy(manifest: *mut MvFpgManifest) -> ();
    pub fn mv_fpg_manifest_count(manifest: *const MvFpgManifest) -> usize;
    pub fn mv_fpg_manifest_transparent_zero(manifest: *const MvFpgManifest) -> c_int;
    pub fn mv_fpg_manifest_palette(manifest: *const MvFpgManifest) -> *const u8;
    pub fn mv_fpg_manifest_gamma(manifest: *const MvFpgManifest) -> *const u8;
    pub fn mv_fpg_manifest_at(
        manifest: *const MvFpgManifest,
        index: usize,
        out: *mut MvFpgManifestSprite,
    ) -> c_int;
    pub fn mv_fpg_manifest_sprite(
        manifest: *const MvFpgManifest,
        id: c_int,
        out: *mut MvFpgManifestSprite,
    ) -> c_int;
    pub fn mv_fpg_manifest_point(
        manifest: *const MvFpgManifest,
        id: c_int,
        index: usize,
        out: *mut MvFpgPoint,
    ) -> c_int;
    pub fn mv_fpg_json_size(fpg: *const MvFpg, transparent_zero: c_int) -> usize;
    pub fn mv_fpg_copy_json(
        fpg: *const MvFpg,
        transparent_zero: c_int,
        output: *mut c_char,
        length: usize,
    ) -> c_int;
}
