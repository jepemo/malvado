// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::window::{MouseButton, Window};
#[derive(Default)]
pub struct Touch {
    down: bool,
    first_x: i32,
    max_x: i32,
    pub complete: bool,
}
impl Touch {
    pub fn tick(&mut self, window: &Window) {
        if self.complete {
            return;
        }
        if window.mouse_down(MouseButton::Left) {
            assert!(
                !window.mouse_down(MouseButton::Middle) && !window.mouse_down(MouseButton::Right)
            );
            let (x, y) = window.mouse_position();
            if !self.down {
                let (width, height) = window.size();
                assert!((x - width / 2).abs() <= 16 && (y - height / 2).abs() <= 16);
                self.first_x = x;
                self.max_x = x;
                self.down = true;
            }
            self.max_x = self.max_x.max(x);
        } else if self.down {
            assert!(self.max_x > self.first_x + 4);
            self.complete = true;
            // SAFETY: the fixture logs this static string and retains nothing.
            unsafe {
                super::acceptance_log(c"rust touch released".as_ptr());
            }
        }
    }
}
