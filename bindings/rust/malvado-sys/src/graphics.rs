// Malvado 2; SPDX-License-Identifier: Apache-2.0
//! Unsafe owner-thread graphics operations from the installed SDK.
use std::ffi::{c_char, c_int};
#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct MvColor {
    pub r: u8,
    pub g: u8,
    pub b: u8,
    pub a: u8,
}
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
