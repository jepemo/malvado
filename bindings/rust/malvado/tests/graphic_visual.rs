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
    use malvado::{Engine, Signal, graphic::Graphic};
    let engine = Engine::new().unwrap();
    let mut actors = Vec::new();
    for color in [
        red,
        Color {
            r: 0,
            g: 255,
            b: 0,
            a: 255,
        },
    ] {
        let actor = engine
            .spawn(Default::default(), |p| async move {
                loop {
                    p.frame().await;
                }
            })
            .unwrap();
        actor.set_position(20.0, 20.0).unwrap();
        actor
            .set_graphic(Graphic {
                geometry: rect,
                color,
            })
            .unwrap();
        actors.push(actor);
    }
    {
        let frame = window.frame(black).unwrap();
        frame.process_graphics(&engine).unwrap();
        pixel(20, 20, (0, 255, 0));
    }
    actors[0].set_z(10.0).unwrap();
    engine.signal(&actors[0], Signal::Freeze).unwrap();
    engine.signal(&actors[1], Signal::Sleep).unwrap();
    {
        let frame = window.frame(black).unwrap();
        frame.process_graphics(&engine).unwrap();
        pixel(20, 20, (255, 0, 0));
    }
    actors[0].set_scale(0.0).unwrap();
    {
        let frame = window.frame(black).unwrap();
        frame.process_graphics(&engine).unwrap();
        pixel(20, 20, (0, 0, 0));
    }
    actors[0].set_scale(1.0).unwrap();
    {
        let frame = window.frame(black).unwrap();
        frame
            .with_clip(
                Rectangle {
                    x: 0,
                    y: 0,
                    width: 5,
                    height: 5,
                },
                |frame| {
                    frame.process_graphics(&engine)?;
                    Ok(())
                },
            )
            .unwrap();
        pixel(20, 20, (0, 0, 0));
        frame.process_graphics(&engine).unwrap();
        pixel(20, 20, (255, 0, 0));
    }
    actors[0].clear_graphic().unwrap();
    {
        let frame = window.frame(black).unwrap();
        frame.process_graphics(&engine).unwrap();
        pixel(20, 20, (0, 0, 0));
    }
    use malvado::camera::{Camera, Selection, Viewport};
    let actor = engine
        .spawn(Default::default(), |p| async move {
            loop {
                p.frame().await;
            }
        })
        .unwrap();
    actor.set_position(60.0, 50.0).unwrap();
    actor
        .set_graphic(Graphic {
            geometry: rect,
            color: red,
        })
        .unwrap();
    engine
        .main_camera()
        .unwrap()
        .set_position(10.0, 5.0)
        .unwrap();
    {
        let frame = window.frame(black).unwrap();
        frame.process_graphics(&engine).unwrap();
        pixel(50, 45, (255, 0, 0));
        pixel(60, 50, (0, 0, 0));
    }
    actor.set_camera(Selection::Screen).unwrap();
    {
        let frame = window.frame(black).unwrap();
        frame.process_graphics(&engine).unwrap();
        pixel(60, 50, (255, 0, 0));
    }
    let camera = engine
        .create_camera(
            Camera::new(
                60.0,
                50.0,
                Some(Viewport {
                    x: 80,
                    y: 10,
                    width: 20,
                    height: 20,
                }),
            )
            .unwrap(),
        )
        .unwrap();
    actor
        .set_camera(Selection::Explicit(camera.clone()))
        .unwrap();
    {
        let frame = window.frame(black).unwrap();
        frame.process_graphics(&engine).unwrap();
        pixel(80, 10, (255, 0, 0));
        pixel(79, 10, (0, 0, 0));
    }
    camera.close().unwrap();
    {
        let frame = window.frame(black).unwrap();
        frame.process_graphics(&engine).unwrap();
        pixel(80, 10, (0, 0, 0));
    }
    actor.set_camera(Selection::Main).unwrap();
    {
        let frame = window.frame(black).unwrap();
        frame
            .with_clip(
                Rectangle {
                    x: 40,
                    y: 35,
                    width: 20,
                    height: 20,
                },
                |frame| {
                    frame.with_scroll((5.0, 5.0), |frame| {
                        frame.process_graphics(&engine)?;
                        pixel(45, 40, (255, 0, 0));
                        Ok(())
                    })
                },
            )
            .unwrap();
    }
    actor.set_scroll((10.0, 5.0)).unwrap();
    actor
        .set_region(Some(Viewport {
            x: 36,
            y: 36,
            width: 3,
            height: 3,
        }))
        .unwrap();
    {
        let frame = window.frame(black).unwrap();
        frame
            .with_clip(
                Rectangle {
                    x: 37,
                    y: 36,
                    width: 5,
                    height: 5,
                },
                |frame| {
                    frame.with_scroll((2.0, 2.0), |frame| {
                        frame.process_graphics(&engine)?;
                        pixel(37, 37, (255, 0, 0));
                        pixel(36, 37, (0, 0, 0));
                        pixel(39, 37, (0, 0, 0));
                        // Native process drawing must restore both host scopes.
                        frame.rectangle(
                            Rectangle {
                                x: 42,
                                y: 40,
                                width: 1,
                                height: 1,
                            },
                            true,
                            red,
                        )?;
                        pixel(40, 38, (255, 0, 0));
                        Ok(())
                    })
                },
            )
            .unwrap();
    }
    actor
        .set_region(Some(Viewport {
            x: 200,
            y: 200,
            width: 10,
            height: 10,
        }))
        .unwrap();
    {
        let frame = window.frame(black).unwrap();
        frame.process_graphics(&engine).unwrap();
        pixel(40, 40, (0, 0, 0));
    }
    actor.set_region(None).unwrap();
    actor.set_scroll((0.0, 0.0)).unwrap();
    {
        let frame = window.frame(black).unwrap();
        frame.process_graphics(&engine).unwrap();
        pixel(50, 45, (255, 0, 0));
    }
    assert_eq!(actor.position().unwrap(), (60.0, 50.0));
}
