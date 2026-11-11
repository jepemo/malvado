// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{
    Error,
    drawing::Rectangle,
    fade::Fade,
    window::{Color, Window},
};
use malvado_sys::graphics as ffi;
unsafe extern "C" {
    fn malvado_test_pixel(x: i32, y: i32, r: i32, g: i32, b: i32) -> i32;
}
fn pixel(x: i32, y: i32, rgb: (i32, i32, i32)) {
    // SAFETY: live main-thread frame; the native helper retains no pointers.
    assert_eq!(
        unsafe { malvado_test_pixel(x, y, rgb.0, rgb.1, rgb.2) },
        1,
        "pixel at {x},{y}: expected {rgb:?}"
    );
}
fn main() {
    let white = Color {
        r: 255,
        g: 255,
        b: 255,
        a: 255,
    };
    let red = Color {
        r: 255,
        g: 0,
        b: 0,
        a: 255,
    };
    let window = Window::open(96, 64, "Rust fade overlay").unwrap();
    let mut fade = Fade::new().unwrap();
    // SAFETY: direct native contract checks on the live owner-thread window.
    assert_eq!(unsafe { ffi::mv_window_draw_fade(white) }, 0);
    let frame = window.frame(white).unwrap();
    frame.fade(&fade).unwrap();
    pixel(2, 2, (255, 255, 255)); // Transparent overlays leave content unchanged.
    fade.enqueue(
        Color {
            a: 128,
            ..Color::default()
        },
        0,
    )
    .unwrap();
    frame
        .with_clip(
            Rectangle {
                x: 10,
                y: 10,
                width: 20,
                height: 20,
            },
            |frame| {
                assert_eq!(frame.fade(&fade), Err(Error::InvalidArgument));
                frame.rectangle(
                    Rectangle {
                        x: 0,
                        y: 0,
                        width: 96,
                        height: 64,
                    },
                    true,
                    red,
                )
            },
        )
        .unwrap();
    pixel(15, 15, (255, 0, 0));
    pixel(5, 15, (255, 255, 255)); // Rejection preserves the active clip.
    frame
        .with_scroll((30.0, 0.0), |frame| {
            assert_eq!(frame.fade(&fade), Err(Error::InvalidArgument));
            frame.rectangle(
                Rectangle {
                    x: 70,
                    y: 10,
                    width: 10,
                    height: 10,
                },
                true,
                red,
            )
        })
        .unwrap();
    pixel(45, 15, (255, 0, 0)); // Rejection preserves the scroll transform.
    frame.fade(&fade).unwrap();
    pixel(15, 15, (127, 0, 0));
    pixel(45, 15, (127, 0, 0));
    pixel(2, 2, (127, 127, 127));
    pixel(93, 61, (127, 127, 127));
    assert_eq!(fade.color().a, 128);
    drop(frame);
    // SAFETY: no active frame; even transparent color must be rejected.
    assert_eq!(unsafe { ffi::mv_window_draw_fade(Color::default()) }, 0);

    fade.enqueue(Color::default(), 2).unwrap();
    assert!(fade.step());
    assert_eq!(fade.color().a, 64);
    for _ in 0..3 {
        let frame = window.frame(white).unwrap();
        frame.fade(&fade).unwrap();
        pixel(2, 2, (191, 191, 191));
        assert!(fade.active()); // Rendering alone never advances the queue.
    }
    assert!(fade.step());
    fade.enqueue(
        Color {
            r: 40,
            g: 80,
            b: 120,
            a: 255,
        },
        0,
    )
    .unwrap();
    let frame = window.frame(white).unwrap();
    frame.text("scene", (5, 5), 20, red).unwrap();
    frame.fade(&fade).unwrap();
    pixel(10, 10, (40, 80, 120));
    pixel(93, 61, (40, 80, 120));
    drop(frame);
    let _frame = window.frame(white).unwrap();
    pixel(10, 10, (255, 255, 255)); // No implicit fade or persistent draw state.
}
