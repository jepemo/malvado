// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::window::{Key, Window};
#[derive(Default)]
pub struct Keyboard {
    index: usize,
    down: bool,
    pressed: bool,
    released: bool,
    announced: bool,
    pub complete: bool,
}
fn log(message: &std::ffi::CStr) {
    // SAFETY: the fixture logs this string synchronously and retains nothing.
    unsafe {
        super::acceptance_log(message.as_ptr());
    }
}
impl Keyboard {
    pub fn tick(&mut self, window: &Window) {
        if self.complete {
            return;
        }
        assert!(
            !window.key_down(Key::B) && !window.key_pressed(Key::B) && !window.key_released(Key::B)
        );
        if self.index == 2 {
            if [Key::A, Key::Left].iter().any(|&key| {
                window.key_down(key) || window.key_pressed(key) || window.key_released(key)
            }) {
                return;
            }
            self.complete = true;
            log(c"rust keyboard passed");
            return;
        }
        let key = [Key::A, Key::Left][self.index];
        let observed = (
            window.key_down(key),
            window.key_pressed(key),
            window.key_released(key),
        );
        assert_eq!(
            observed,
            (
                window.key_down(key),
                window.key_pressed(key),
                window.key_released(key)
            )
        );
        self.down |= observed.0;
        self.pressed |= observed.1;
        self.released |= observed.2;
        if self.down && self.pressed && !self.announced {
            self.announced = true;
            log(if self.index == 0 {
                c"rust keyboard a held"
            } else {
                c"rust keyboard left held"
            });
        }
        if self.down && self.pressed && self.released && !observed.0 {
            log(if self.index == 0 {
                c"rust keyboard a released"
            } else {
                c"rust keyboard left released"
            });
            self.index += 1;
            self.announced = false;
            self.down = false;
            self.pressed = false;
            self.released = false;
        }
    }
}
