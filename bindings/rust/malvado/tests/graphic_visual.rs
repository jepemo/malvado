// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{
    Error,
    collision::Transform,
    drawing::Rectangle,
    graphic::{Geometry, Pivot},
    window::{Color, Window},
};
use malvado_sys::{self as sys, graphics as ffi};
unsafe extern "C" {
    fn malvado_test_pixel(x: i32, y: i32, r: i32, g: i32, b: i32) -> i32;
}
fn pixel(x: i32, y: i32, rgb: (i32, i32, i32)) {
    // SAFETY: a live owner-thread frame; test helper retains nothing.
    assert_eq!(
        unsafe { malvado_test_pixel(x, y, rgb.0, rgb.1, rgb.2) },
        1,
        "pixel {x},{y}"
    );
}
fn main() {
    let black = Color {
        r: 0,
        g: 0,
        b: 0,
        a: 255,
    };
    let red = Color {
        r: 255,
        g: 0,
        b: 0,
        a: 255,
    };
    let rect = Geometry::rectangle(10.0, 10.0, Pivot::Center).unwrap();
    let corner = Geometry::rectangle(10.0, 10.0, Pivot::TopLeft).unwrap();
    let circle = Geometry::circle(5.0).unwrap();
    let window = Window::open(120, 100, "Rust graphic geometry").unwrap();
    let mut raw = sys::MvCollisionShape::default();
    // SAFETY: output lives for the call; direct native checks stay on the owner thread.
    assert_eq!(unsafe { sys::mv_graphic_circle(5.0, &mut raw) }, 1);
    assert_eq!(
        unsafe {
            ffi::mv_window_draw_graphic(
                raw,
                sys::MvCollisionTransform {
                    x: 0.0,
                    y: 0.0,
                    angle: 0.0,
                    scale: 1.0,
                },
                red,
            )
        },
        0
    );
    for _ in 0..2 {
        let frame = window.frame(black).unwrap();
        frame
            .graphic(
                rect,
                Transform {
                    x: 20.0,
                    y: 20.0,
                    scale: 2.0,
                    ..Default::default()
                },
                red,
            )
            .unwrap();
        pixel(12, 12, (255, 0, 0));
        pixel(8, 8, (0, 0, 0));
        frame
            .graphic(
                corner,
                Transform {
                    x: 60.0,
                    y: 20.0,
                    angle: 90.0,
                    ..Default::default()
                },
                red,
            )
            .unwrap();
        pixel(55, 25, (255, 0, 0));
        pixel(65, 25, (0, 0, 0));
        frame
            .graphic(
                circle,
                Transform {
                    x: 90.0,
                    y: 20.0,
                    scale: 2.0,
                    ..Default::default()
                },
                red,
            )
            .unwrap();
        pixel(90, 20, (255, 0, 0));
        pixel(102, 20, (0, 0, 0));
        for scale in [0.0, -1.0, f64::NAN, f64::INFINITY, f64::MAX, 1e-300] {
            assert_eq!(
                frame.graphic(
                    rect,
                    Transform {
                        x: 10.0,
                        y: 70.0,
                        scale,
                        ..Default::default()
                    },
                    red
                ),
                Err(Error::InvalidArgument)
            );
        }
        assert!(
            frame
                .graphic(
                    rect,
                    Transform {
                        x: f64::MAX,
                        ..Default::default()
                    },
                    red
                )
                .is_err()
        );
        assert!(
            frame
                .graphic(
                    rect,
                    Transform {
                        angle: f64::NAN,
                        ..Default::default()
                    },
                    red
                )
                .is_err()
        );
        pixel(10, 70, (0, 0, 0));
        // Tampered C geometry must also fail without drawing.
        raw.offset_x = 1.0;
        assert_eq!(
            unsafe {
                ffi::mv_window_draw_graphic(
                    raw,
                    sys::MvCollisionTransform {
                        x: 10.0,
                        y: 70.0,
                        angle: 0.0,
                        scale: 1.0,
                    },
                    red,
                )
            },
            0
        );
        frame
            .with_clip(
                Rectangle {
                    x: 10,
                    y: 60,
                    width: 20,
                    height: 20,
                },
                |frame| {
                    frame.with_scroll((10.0, 0.0), |frame| {
                        frame.graphic(
                            corner,
                            Transform {
                                x: 25.0,
                                y: 65.0,
                                scale: 3.0,
                                ..Default::default()
                            },
                            red,
                        )
                    })
                },
            )
            .unwrap();
        pixel(20, 70, (255, 0, 0));
        pixel(35, 70, (0, 0, 0));
        frame
            .graphic(
                circle,
                Transform {
                    x: 90.0,
                    y: 70.0,
                    ..Default::default()
                },
                red,
            )
            .unwrap();
        pixel(90, 70, (255, 0, 0));
    }
}
