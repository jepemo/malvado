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
    ];
    assert!(native.len() == rust.len(), "graphics ABI count mismatch");
    let mut i = 0;
    while i < rust.len() {
        assert!(native[i] == rust[i], "graphics ABI layout/value mismatch");
        i += 1;
    }
};
