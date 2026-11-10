// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{
    Error,
    drawing::Rectangle,
    window::{Color, Window},
};
unsafe extern "C" {
    fn malvado_test_pixel(x: i32, y: i32, r: i32, g: i32, b: i32) -> i32;
}
fn pixel(x: i32, y: i32, color: Color) {
    // SAFETY: tests call only during a live main-thread frame; helper retains nothing.
    assert_eq!(
        unsafe { malvado_test_pixel(x, y, color.r.into(), color.g.into(), color.b.into()) },
        1,
        "pixel at {x},{y}"
    );
}
fn main() {
    let window = Window::open(160, 120, "Rust drawing").unwrap();
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
    let green = Color {
        r: 0,
        g: 255,
        b: 0,
        a: 255,
    };
    let blue = Color {
        r: 0,
        g: 0,
        b: 255,
        a: 255,
    };
    let area = Rectangle {
        x: 10,
        y: 10,
        width: 20,
        height: 20,
    };
    assert!(window.text_width("Hello", 20).unwrap() > 0);
    assert_eq!(window.text_width("", 20).unwrap(), 0);
    assert_eq!(
        window.text_width("bad\0text", 20),
        Err(Error::InvalidArgument)
    );
    let frame = window.frame(black).unwrap();
    assert!(
        frame
            .rectangle(Rectangle { width: 0, ..area }, true, red)
            .is_err()
    );
    assert!(frame.circle((i32::MAX, 0), 1, true, red).is_err());
    assert!(frame.filled_circle((f32::NAN, 0.0), 1.0, red).is_err());
    assert!(
        frame
            .transformed_rectangle((0.0, 0.0), (0.0, 1.0), (0.0, 0.0), 0.0, red)
            .is_err()
    );
    assert!(frame.text("bad\0text", (0, 0), 20, red).is_err());
    assert!(frame.text("Hello", (0, 0), 0, red).is_err());
    assert!(
        frame
            .with_clip(Rectangle { x: -1, ..area }, |_| Ok(()))
            .is_err()
    );
    frame
        .with_clip(area, |frame| {
            assert!(frame.with_clip(area, |_| Ok(())).is_err());
            frame.with_scroll((5.0, 5.0), |frame| {
                assert!(frame.with_clip(area, |_| Ok(())).is_err());
                assert!(frame.with_scroll((0.0, 0.0), |_| Ok(())).is_err());
                frame.rectangle(
                    Rectangle {
                        x: 0,
                        y: 0,
                        width: 100,
                        height: 100,
                    },
                    true,
                    red,
                )
            })
        })
        .unwrap();
    pixel(15, 15, red);
    pixel(5, 15, black);
    frame.circle((50, 20), 8, true, green).unwrap();
    frame.circle((50, 20), 10, false, green).unwrap();
    frame
        .transformed_rectangle((70.0, 20.0), (10.0, 10.0), (5.0, 5.0), 45.0, blue)
        .unwrap();
    frame.filled_circle((90.0, 20.0), 8.0, green).unwrap();
    pixel(50, 20, green);
    pixel(70, 20, blue);
    pixel(90, 20, green);
    let unwind = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        let _: Result<(), Error> = frame.with_clip(area, |frame| {
            frame.with_scroll((5.0, 5.0), |_| panic!("scope cleanup"))
        });
    }));
    assert!(unwind.is_err());
    assert_eq!(
        frame.with_clip(area, |_| Err::<(), _>(Error::Process)),
        Err(Error::Process)
    );
    frame.with_scroll((0.0, 0.0), |_| Ok(())).unwrap();
    frame
        .rectangle(
            Rectangle {
                x: 110,
                y: 50,
                width: 10,
                height: 10,
            },
            true,
            blue,
        )
        .unwrap();
    pixel(115, 55, blue);
    frame.text("Hello", (5, 80), 20, green).unwrap();
    drop(frame);
    drop(window.frame(black).unwrap());
}
