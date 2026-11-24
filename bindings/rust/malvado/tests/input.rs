// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::window::{Color, MouseButton, PadAxis, PadButton, Window};
unsafe extern "C" {
    fn mv_text_input_sample(codepoint: u32);
    fn mv_mouse_sample(button: i32, pressed: i32, released: i32);
    fn mv_pad_sample(slot: i32, connected: i32, held: u32);
}
fn sample(value: u32) {
    // SAFETY: private verification seam, called only on the window owner thread.
    unsafe {
        mv_text_input_sample(value);
    }
}
fn neutral_and_unicode() {
    let buttons = [
        PadButton::A,
        PadButton::B,
        PadButton::X,
        PadButton::Y,
        PadButton::Up,
        PadButton::Down,
        PadButton::Left,
        PadButton::Right,
        PadButton::LeftBumper,
        PadButton::RightBumper,
        PadButton::Back,
        PadButton::Start,
    ];
    let axes = [
        PadAxis::LeftX,
        PadAxis::LeftY,
        PadAxis::RightX,
        PadAxis::RightY,
        PadAxis::LeftTrigger,
        PadAxis::RightTrigger,
    ];
    for _ in 0..2 {
        let window = Window::open(160, 120, "Rust native input").unwrap();
        assert_eq!(window.text_input_next(), None);
        if !cfg!(target_os = "android") {
            assert!(!window.set_text_input_keyboard(true));
        }
        for slot in [0, 1, 2, 3, 4, 255] {
            if !window.pad_connected(slot) {
                for button in buttons {
                    assert!(!window.pad_button_down(slot, button));
                    assert!(!window.pad_button_pressed(slot, button));
                    assert!(!window.pad_button_released(slot, button));
                }
                for axis in axes {
                    assert_eq!(window.pad_axis(slot, axis), 0.0);
                }
            }
            for axis in axes {
                let range = match axis {
                    PadAxis::LeftTrigger | PadAxis::RightTrigger => 0.0..=1.0,
                    _ => -1.0..=1.0,
                };
                assert!(range.contains(&window.pad_axis(slot, axis)));
            }
        }
        let _ = window.mouse_position();
        assert!(!window.mouse_pressed(MouseButton::Left));
        assert!(!window.mouse_released(MouseButton::Left));
        // SAFETY: private capture seams on the window owner thread.
        unsafe {
            mv_mouse_sample(MouseButton::Left as i32, 1, 0);
            mv_mouse_sample(MouseButton::Left as i32, 0, 1);
            mv_pad_sample(3, 1, 0);
            mv_pad_sample(3, 1, 1 << (PadButton::A as u32));
        }
        window.input_tick();
        for reader in [&window, &window] {
            assert!(reader.mouse_pressed(MouseButton::Left));
            assert!(reader.mouse_released(MouseButton::Left));
            assert!(reader.pad_button_pressed(3, PadButton::A));
            assert!(!reader.pad_button_pressed(0, PadButton::A));
        }
        window.input_tick();
        assert!(!window.mouse_pressed(MouseButton::Left));
        assert!(!window.pad_button_pressed(3, PadButton::A));
        for invalid in [0, 0xd800, 0xdfff, 0x110000, u32::MAX] {
            sample(invalid);
        }
        for value in ['a', 'Ω', '€', '😀', '\u{10ffff}'] {
            sample(value.into());
        }
        let (first_reader, second_reader) = (&window, &window);
        assert_eq!(first_reader.text_input_next(), Some('a'));
        for value in ['Ω', '€', '😀', '\u{10ffff}'] {
            assert_eq!(second_reader.text_input_next(), Some(value));
        }
        assert_eq!(window.text_input_next(), None);
        for _ in 0..300 {
            sample('q'.into());
        }
        for _ in 0..256 {
            assert_eq!(window.text_input_next(), Some('q'));
        }
        assert_eq!(window.text_input_next(), None);
        sample('x'.into()); // Close must clear an unconsumed scalar.
    }
}
fn mouse() {
    let window = Window::open(160, 120, "Malvado Rust mouse").unwrap();
    let buttons = [MouseButton::Left, MouseButton::Middle, MouseButton::Right];
    let mut phase = 0;
    let mut presses = [0; 3];
    let mut releases = [0; 3];
    for _ in 0..600 {
        window.input_tick();
        for (index, &button) in buttons.iter().enumerate() {
            presses[index] += u8::from(window.mouse_pressed(button));
            releases[index] += u8::from(window.mouse_released(button));
        }
        let held = buttons
            .iter()
            .filter(|&&button| window.mouse_down(button))
            .count();
        if phase == 0 && window.mouse_position() == (50, 45) && held == 3 {
            phase = 1;
        } else if phase == 1 && held == 0 {
            phase = 2;
            break;
        }
        drop(window.frame(Color::default()).unwrap());
    }
    assert_eq!(phase, 2);
    assert_eq!(presses, [1; 3]);
    assert_eq!(releases, [1; 3]);
}
fn text() {
    let window = Window::open(160, 120, "Malvado Rust text input").unwrap();
    let mut text = String::new();
    for frame in 0..600 {
        if frame >= 60 {
            while let Some(value) = window.text_input_next() {
                text.push(value);
            }
            if text.len() >= 3 {
                break;
            }
        }
        drop(window.frame(Color::default()).unwrap());
    }
    assert_eq!(text, "aZ9");
    assert_eq!(window.text_input_next(), None);
}
fn main() {
    if std::env::args().any(|arg| arg == "--mouse") {
        mouse();
    } else if std::env::args().any(|arg| arg == "--text") {
        text();
    } else {
        neutral_and_unicode();
    }
}
