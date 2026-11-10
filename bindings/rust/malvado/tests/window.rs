// Malvado 2; SPDX-License-Identifier: Apache-2.0
//! Bounded owner-thread executable; requires a real or virtual display.
use malvado::{
    Error,
    window::{Color, Window},
};
fn main() {
    std::thread::spawn(|| {
        assert!(matches!(
            Window::open(160, 120, "wrong thread"),
            Err(Error::Platform(_))
        ));
    })
    .join()
    .unwrap();
    assert!(matches!(
        Window::open(0, 120, "invalid"),
        Err(Error::InvalidArgument)
    ));
    assert!(matches!(
        Window::open(160, 120, "bad\0title"),
        Err(Error::InvalidArgument)
    ));
    for _ in 0..2 {
        let window = Window::open(160, 120, "Malvado Rust window").unwrap();
        assert_eq!(window.size(), (160, 120));
        window.set_title("Malvado Rust window renamed").unwrap();
        assert_eq!(window.set_title("bad\0title"), Err(Error::InvalidArgument));
        assert!(matches!(Window::open(160, 120, "other"), Err(Error::Busy)));
        assert!(!window.should_close());
        let frame = window
            .frame(Color {
                r: 10,
                g: 20,
                b: 30,
                a: 255,
            })
            .unwrap();
        assert!(matches!(window.frame(Color::default()), Err(Error::Busy)));
        drop(frame);
        drop(window.frame(Color::default()).unwrap());
        let unwound = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
            let _frame = window.frame(Color::default()).unwrap();
            panic!("verify frame cleanup during unwind");
        }));
        assert!(unwound.is_err());
        drop(window.frame(Color::default()).unwrap());
        // Forget is safe: owner cleanup must finish the active native frame.
        std::mem::forget(window.frame(Color::default()).unwrap());
    }
}
