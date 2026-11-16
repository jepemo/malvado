// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{
    Error, Game, SpawnOptions,
    drawing::Rectangle,
    window::{Color, Window},
};
use std::{cell::Cell, rc::Rc};
unsafe extern "C" {
    fn acceptance_log(message: *const std::ffi::c_char);
    fn acceptance_pixel() -> i32;
}
struct Guard(Rc<Cell<usize>>);
impl Drop for Guard {
    fn drop(&mut self) {
        self.0.set(self.0.get() + 1);
    }
}
#[unsafe(no_mangle)]
pub extern "C" fn malvado_rust_entry() {
    std::thread::spawn(|| {
        assert!(matches!(
            Window::open(320, 240, "wrong thread"),
            Err(Error::Platform(_))
        ));
    })
    .join()
    .unwrap();
    let window = Window::open(320, 240, "Malvado Android Rust").unwrap();
    // SAFETY: the C fixture logs this static string and retains no pointer.
    unsafe {
        acceptance_log(c"window opened".as_ptr());
    }
    let (width, height) = window.size();
    assert!(width > 0 && height > 0);
    let drops = Rc::new(Cell::new(0));
    let guard = Guard(drops.clone());
    let mut game = Game::new().unwrap();
    game.spawn(SpawnOptions::default(), move |process| async move {
        let _guard = guard;
        loop {
            process.frame().await;
        }
    })
    .unwrap();
    let mut verified = false;
    for index in 0..90 {
        game.tick().unwrap();
        let frame = window
            .frame(Color {
                r: 20,
                g: 40,
                b: 80,
                a: 255,
            })
            .unwrap();
        frame
            .rectangle(
                Rectangle {
                    x: width / 4,
                    y: height / 4,
                    width: width / 2,
                    height: height / 2,
                },
                true,
                Color {
                    r: 255,
                    g: 0,
                    b: 0,
                    a: 255,
                },
            )
            .unwrap();
        if index == 30 {
            // SAFETY: a live owner-thread frame supplies the current framebuffer.
            assert_eq!(unsafe { acceptance_pixel() }, 1);
            verified = true;
        }
        drop(frame);
    }
    assert!(verified);
    drop(game);
    assert_eq!(drops.get(), 1);
    drop(window);
    // SAFETY: the C fixture logs this static string and retains no pointer.
    unsafe {
        acceptance_log(c"Rust scheduler and cleanup passed".as_ptr());
    }
}
