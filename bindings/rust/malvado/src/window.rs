// Malvado 2; SPDX-License-Identifier: Apache-2.0
//! One owner-thread window. The caller controls simulation and render timing.
use crate::Error;
use malvado_sys::{self as sys, graphics as ffi};
use std::{
    cell::{Cell, RefCell},
    ffi::CString,
    marker::PhantomData,
    rc::Rc,
    sync::atomic::{AtomicBool, Ordering},
};

pub use ffi::MvColor as Color;
static WINDOW_OWNED: AtomicBool = AtomicBool::new(false);

/// The only safe Rust window owner. Open it on the application main thread.
/// It is independent of Engine; dropping it closes the native window.
/// Do not open/close the same native window through another host while owned.
pub struct Window {
    drawing: Cell<bool>,
    local: PhantomData<Rc<()>>,
}
impl Window {
    pub fn open(width: i32, height: i32, title: &str) -> Result<Self, Error> {
        sys::check_abi().map_err(Error::Abi)?;
        if width <= 0 || height <= 0 {
            return Err(Error::InvalidArgument);
        }
        let title = CString::new(title).map_err(|_| Error::InvalidArgument)?;
        if !main_thread() {
            return Err(Error::Platform(
                "window requires the application main thread",
            ));
        }
        WINDOW_OWNED
            .compare_exchange(false, true, Ordering::AcqRel, Ordering::Acquire)
            .map_err(|_| Error::Busy)?;
        // SAFETY: this thread acquired the single window lease. C copies title.
        if unsafe { ffi::mv_window_open(width, height, title.as_ptr()) } == 0 {
            WINDOW_OWNED.store(false, Ordering::Release);
            return Err(Error::Platform("window open failed"));
        }
        Ok(Self {
            drawing: Cell::new(false),
            local: PhantomData,
        })
    }
    pub fn set_title(&self, title: &str) -> Result<(), Error> {
        let title = CString::new(title).map_err(|_| Error::InvalidArgument)?;
        // SAFETY: owner thread, live window and terminated string.
        if unsafe { ffi::mv_window_set_title(title.as_ptr()) } == 0 {
            return Err(Error::Platform("window title failed"));
        }
        Ok(())
    }
    pub fn size(&self) -> (i32, i32) {
        let (mut width, mut height) = (0, 0);
        // SAFETY: distinct stack outputs and owner-thread window.
        unsafe {
            ffi::mv_window_size(&mut width, &mut height);
        }
        (width, height)
    }
    pub fn should_close(&self) -> bool {
        // SAFETY: live owner-thread window.
        unsafe { ffi::mv_window_should_close() != 0 }
    }
    /// Begin one render frame. Dropping its guard ends drawing and polls events.
    /// Nested frames return Busy. This does not advance the process scheduler.
    pub fn frame(&self, background: Color) -> Result<Frame<'_>, Error> {
        if self.drawing.replace(true) {
            return Err(Error::Busy);
        }
        // SAFETY: this window owns the only active frame.
        unsafe {
            ffi::mv_window_begin(background);
        }
        Ok(Frame {
            window: self,
            images: RefCell::new(Vec::new()),
        })
    }
}
impl Drop for Window {
    fn drop(&mut self) {
        // SAFETY: the unique window owner runs on its creating thread. Finish
        // a forgotten frame before closing; ordinary guards already ended it.
        unsafe {
            if self.drawing.replace(false) {
                ffi::mv_window_end();
            }
            ffi::mv_window_close();
        }
        WINDOW_OWNED.store(false, Ordering::Release);
    }
}
/// A live render frame borrows its window and cannot outlive it.
pub struct Frame<'a> {
    pub(crate) window: &'a Window,
    pub(crate) images: RefCell<Vec<crate::image::Image>>,
}
impl Drop for Frame<'_> {
    fn drop(&mut self) {
        // SAFETY: the borrowed window remains live; only one guard can exist.
        unsafe {
            ffi::mv_window_end();
        }
        self.window.drawing.set(false);
    }
}
/// Window and frame ownership stay on the creating thread.
/// ```compile_fail
/// use malvado::window::Window;
/// fn invalid(window: Window) { std::thread::spawn(move || drop(window)); }
/// ```
/// A frame cannot outlive its window.
/// ```compile_fail
/// use malvado::window::{Window, Color};
/// let frame = {
///     let window = Window::open(160, 120, "game").unwrap();
///     window.frame(Color::default()).unwrap()
/// };
/// drop(frame);
/// ```
mod ownership_contract {}

#[cfg(target_os = "macos")]
fn main_thread() -> bool {
    unsafe extern "C" {
        fn pthread_main_np() -> std::ffi::c_int;
    }
    // SAFETY: this system query has no pointers or initialization requirement.
    unsafe { pthread_main_np() != 0 }
}
#[cfg(target_os = "linux")]
fn main_thread() -> bool {
    unsafe extern "C" {
        fn gettid() -> std::ffi::c_int;
    }
    // SAFETY: the Linux thread ID query has no arguments or retained state.
    unsafe { gettid() as u32 == std::process::id() }
}

/// Supported physical keys. Values are checked against the installed C enum.
#[repr(i32)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Key {
    Digit0 = ffi::MV_KEY_0,
    Digit1 = ffi::MV_KEY_1,
    Digit2 = ffi::MV_KEY_2,
    Digit3 = ffi::MV_KEY_3,
    Digit4 = ffi::MV_KEY_4,
    Digit5 = ffi::MV_KEY_5,
    Digit6 = ffi::MV_KEY_6,
    Digit7 = ffi::MV_KEY_7,
    Digit8 = ffi::MV_KEY_8,
    Digit9 = ffi::MV_KEY_9,
    A = ffi::MV_KEY_A,
    B = ffi::MV_KEY_B,
    C = ffi::MV_KEY_C,
    D = ffi::MV_KEY_D,
    E = ffi::MV_KEY_E,
    F = ffi::MV_KEY_F,
    G = ffi::MV_KEY_G,
    H = ffi::MV_KEY_H,
    I = ffi::MV_KEY_I,
    J = ffi::MV_KEY_J,
    K = ffi::MV_KEY_K,
    L = ffi::MV_KEY_L,
    M = ffi::MV_KEY_M,
    N = ffi::MV_KEY_N,
    O = ffi::MV_KEY_O,
    P = ffi::MV_KEY_P,
    Q = ffi::MV_KEY_Q,
    R = ffi::MV_KEY_R,
    S = ffi::MV_KEY_S,
    T = ffi::MV_KEY_T,
    U = ffi::MV_KEY_U,
    V = ffi::MV_KEY_V,
    W = ffi::MV_KEY_W,
    X = ffi::MV_KEY_X,
    Y = ffi::MV_KEY_Y,
    Z = ffi::MV_KEY_Z,
    Escape = ffi::MV_KEY_ESCAPE,
    Space = ffi::MV_KEY_SPACE,
    Left = ffi::MV_KEY_LEFT,
    Right = ffi::MV_KEY_RIGHT,
    Up = ffi::MV_KEY_UP,
    Down = ffi::MV_KEY_DOWN,
    Enter = ffi::MV_KEY_ENTER,
    Tab = ffi::MV_KEY_TAB,
    Backspace = ffi::MV_KEY_BACKSPACE,
}
impl Window {
    /// Commit buffered transitions once before a shared logical simulation tick.
    /// Do not call this per process. Reads never consume a transition.
    pub fn input_tick(&self) {
        // SAFETY: live owner-thread window, native global snapshot update.
        unsafe {
            ffi::mv_input_tick();
        }
    }
    pub fn key_down(&self, key: Key) -> bool {
        // SAFETY: typed supported key and live owner-thread window.
        unsafe { ffi::mv_key_down(key as i32) != 0 }
    }
    pub fn key_pressed(&self, key: Key) -> bool {
        // SAFETY: reads the shared native snapshot without mutation.
        unsafe { ffi::mv_key_pressed(key as i32) != 0 }
    }
    pub fn key_released(&self, key: Key) -> bool {
        // SAFETY: reads the shared native snapshot without mutation.
        unsafe { ffi::mv_key_released(key as i32) != 0 }
    }
}
