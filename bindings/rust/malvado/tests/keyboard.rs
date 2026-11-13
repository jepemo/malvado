// Malvado 2; SPDX-License-Identifier: Apache-2.0
//! Main-thread keyboard checks. Private injection is only a verification fixture.
use malvado::{
    Game, SpawnOptions,
    window::{Color, Key, Window},
};
use std::{cell::Cell, rc::Rc};
unsafe extern "C" {
    fn mv_keyboard_sample(key: i32, pressed: i32, released: i32);
}
fn inject(tick: usize) {
    // SAFETY: test runs on the window owner thread. This private native helper
    // inserts observations only; public queries still use the real native state.
    unsafe {
        if tick == 1 || tick == 4 {
            mv_keyboard_sample(Key::Space as i32, 1, 0);
        }
        if tick == 3 || tick == 4 {
            mv_keyboard_sample(Key::Space as i32, 0, 1);
        }
    }
}
fn snapshots() {
    for _ in 0..2 {
        let window = Rc::new(Window::open(160, 120, "Rust keyboard snapshots").unwrap());
        assert!(!window.key_pressed(Key::Space) && !window.key_released(Key::Space));
        let tick = Rc::new(Cell::new(0));
        let calls = Rc::new(Cell::new(0));
        let mut game = Game::new().unwrap();
        for rate in [60, 60, 30] {
            let (window, tick, calls) = (window.clone(), tick.clone(), calls.clone());
            game.spawn(
                SpawnOptions {
                    fps: rate,
                    ..Default::default()
                },
                move |process| async move {
                    let expected: &[usize] = if rate == 30 {
                        &[1, 3, 5]
                    } else {
                        &[1, 2, 3, 4, 5]
                    };
                    for &number in expected {
                        assert_eq!(tick.get(), number);
                        assert_eq!(window.key_pressed(Key::Space), number == 2 || number == 5);
                        assert_eq!(window.key_pressed(Key::Space), number == 2 || number == 5);
                        assert_eq!(window.key_released(Key::Space), number == 4 || number == 5);
                        if number < 5 {
                            process.frame().await;
                        }
                    }
                    calls.set(calls.get() + 1);
                    Ok(())
                },
            )
            .unwrap();
        }
        for number in 1..=5 {
            tick.set(number);
            window.input_tick();
            game.tick().unwrap();
            println!(
                "rust-key: {} {} {}",
                number,
                u8::from(window.key_pressed(Key::Space)),
                u8::from(window.key_released(Key::Space))
            );
            drop(window.frame(Color::default()).unwrap());
            inject(number);
        }
        assert_eq!(calls.get(), 3);
        assert_eq!(game.process_count(), 0);
        window.input_tick(); // Catch-up clears committed edges without replay.
        assert!(!window.key_pressed(Key::Space) && !window.key_released(Key::Space));
        inject(1);
        for _ in 0..3 {
            drop(window.frame(Color::default()).unwrap());
        }
        window.input_tick(); // Frames with no tick retained the pending press.
        assert!(window.key_pressed(Key::Space));
        assert!(!window.key_down(Key::Space)); // Synthetic press does not fake held state.
        inject(4); // Both pending and committed edges must reset at close.
        drop(game);
        assert_eq!(Rc::strong_count(&window), 1);
        drop(window);
        // SAFETY: no window is open, so raw native queries must be neutral.
        unsafe {
            assert_eq!(malvado_sys::graphics::mv_key_pressed(Key::Space as i32), 0);
            assert_eq!(malvado_sys::graphics::mv_key_released(Key::Space as i32), 0);
        }
    }
}
fn physical() {
    let window = Window::open(160, 120, "Malvado Rust keys").unwrap();
    let keys = [
        Key::Left,
        Key::Right,
        Key::Up,
        Key::Down,
        Key::A,
        Key::Digit0,
        Key::Space,
    ];
    let (mut pressed, mut released, mut phase) = (0, 0, 0);
    for _ in 0..600 {
        window.input_tick();
        let held = keys.iter().filter(|&&key| window.key_down(key)).count();
        for &key in &keys {
            let edge = window.key_pressed(key);
            assert_eq!(edge, window.key_pressed(key));
            pressed += u32::from(edge);
            released += u32::from(window.key_released(key));
        }
        if phase == 0 && held == keys.len() {
            phase = 1;
        } else if phase == 1 && held == 0 {
            phase = 2;
        } else if phase == 2 && window.key_pressed(Key::Escape) {
            phase = 3;
            break;
        }
        window.input_tick();
        for &key in &keys {
            assert!(!window.key_pressed(key) && !window.key_released(key));
        }
        drop(window.frame(Color::default()).unwrap());
    }
    assert_eq!((pressed, released, phase), (7, 7, 3));
}
fn main() {
    if std::env::args().any(|a| a == "--physical") {
        physical();
    } else {
        snapshots();
    }
}
