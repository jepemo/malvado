// Malvado 2; SPDX-License-Identifier: Apache-2.0
#![cfg(feature = "graphics")]
use malvado::{Error, fade::Fade, window::Color};
const WHITE: Color = Color {
    r: 255,
    g: 255,
    b: 255,
    a: 255,
};
#[test]
fn native_fifo_matches_c_and_lua_trace() {
    let mut fade = Fade::new().unwrap();
    fade.enqueue(
        Color {
            a: 255,
            ..Color::default()
        },
        2,
    )
    .unwrap();
    fade.enqueue(
        Color {
            r: 20,
            g: 40,
            b: 60,
            a: 80,
        },
        0,
    )
    .unwrap();
    fade.enqueue(
        Color {
            r: 0,
            g: 20,
            b: 40,
            a: 0,
        },
        2,
    )
    .unwrap();
    fade.enqueue(
        Color {
            r: 1,
            g: 2,
            b: 3,
            a: 4,
        },
        0,
    )
    .unwrap();
    let mut trace = String::new();
    for tick in 0..5 {
        let Color { r, g, b, a } = fade.color();
        trace.push_str(&format!(
            "{tick} {r} {g} {b} {a} {}\n",
            u8::from(fade.active())
        ));
        fade.step();
    }
    assert_eq!(trace, include_str!("fixtures/fade.txt"));
}
#[test]
fn queue_errors_wraparound_and_copied_state() {
    let mut fade = Fade::new().unwrap();
    assert!(!fade.active() && !fade.step());
    assert_eq!(fade.enqueue(WHITE, 1_000_001), Err(Error::InvalidArgument));
    assert_eq!(fade.color(), Color::default());
    for _ in 0..40 {
        fade.enqueue(WHITE, 1).unwrap();
        for value in 0..16 {
            fade.enqueue(Color { a: value, ..WHITE }, 0).unwrap();
        }
        assert_eq!(fade.enqueue(WHITE, 0), Err(Error::QueueFull));
        assert_eq!(fade.enqueue(WHITE, u32::MAX), Err(Error::InvalidArgument));
        assert!(fade.step());
        assert_eq!(fade.color().a, 15);
        assert!(!fade.active() && !fade.step());
    }
    fade.enqueue(WHITE, 1_000_000).unwrap();
    fade = Fade::new().unwrap();
    fade.enqueue(WHITE, 2).unwrap();
    let copy = fade;
    assert!(fade.step());
    assert_eq!(copy.color().a, 0);
    assert_eq!(fade.color().a, 127);
    fade.enqueue(Color::default(), 0).unwrap();
    assert!(copy.active());
}
