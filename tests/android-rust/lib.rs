// Malvado 2; SPDX-License-Identifier: Apache-2.0
mod assets;
mod audio;
mod failures;
mod keyboard;
mod music;
mod touch;
use malvado::{
    Error, Game,
    drawing::Rectangle,
    window::{Color, Window},
};
use std::{cell::Cell, rc::Rc};
unsafe extern "C" {
    fn acceptance_log(message: *const std::ffi::c_char);
    fn acceptance_pixel() -> i32;
    fn acceptance_mode() -> i32;
    fn acceptance_audio() -> i32;
    fn acceptance_failure() -> i32;
    fn acceptance_lifecycle_frame(verified: i32);
    fn acceptance_lifecycle_finish();
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
    // SAFETY: this reads immutable fixture configuration on the app thread.
    let failure = unsafe { acceptance_failure() };
    if failure == 3 {
        assert!(matches!(Window::open(320, 240, "Cancelled startup"), Err(Error::Platform(_))));
        return;
    }
    let window = Rc::new(Window::open(320, 240, "Malvado Android Rust").unwrap());
    let weak = Rc::downgrade(&window);
    let (snapshot, image) = assets::load(window.clone());
    if failure != 0 {
        failures::run(window, snapshot, image, failure);
        return;
    }
    let image = image.unwrap();
    let owned_image = image.clone();
    // SAFETY: the C fixture logs this static string and retains no pointer.
    unsafe {
        acceptance_log(c"window opened".as_ptr());
    }
    let (width, height) = window.size();
    assert!(width > 0 && height > 0);
    let drops = Rc::new(Cell::new(0));
    let guard = Guard(drops.clone());
    let mut game = Game::new().unwrap();
    game.spawn(move |process| async move {
        let _guard = guard;
        let _image = owned_image;
        loop {
            process.frame().await;
        }
    })
    .unwrap();
    // SAFETY: fixture configuration is immutable and this is the app thread.
    let mode = unsafe { acceptance_mode() };
    // SAFETY: immutable fixture flag on the application thread.
    let audio = if unsafe { acceptance_audio() } != 0 {
        Some(audio::Case::begin())
    } else {
        None
    };
    if let Some(audio) = &audio {
        audio.attach(&mut game);
    }
    let mut keyboard = keyboard::Keyboard::default();
    let mut touch = touch::Touch::default();
    let mut verified = false;
    let mut expected_text = "aZ0".chars();
    for index in 0..if mode == 0 { 90 } else { 3600 } {
        if window.should_close() {
            break;
        }
        if let Some(audio) = &audio {
            audio.tick();
        }
        window.input_tick();
        if mode == 4 || mode == 5 || mode == 6 {
            keyboard.tick(&window);
        }
        if mode == 3 || mode == 7 {
            touch.tick(&window);
        }
        if mode == 9 {
            while let Some(character) = window.text_input_next() {
                assert_eq!(expected_text.next(), Some(character));
                if expected_text.as_str().is_empty() {
                    // SAFETY: the fixture logs this static string synchronously.
                    unsafe { acceptance_log(c"rust text input passed".as_ptr()) };
                }
            }
        }
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
        // SAFETY: the fixture observes native lifecycle events during a live frame.
        unsafe {
            acceptance_lifecycle_frame(i32::from(verified));
        }
        drop(frame);
    }
    assert!(verified);
    if mode == 9 {
        assert!(expected_text.as_str().is_empty());
    }
    if mode != 0 {
        assert!(window.should_close());
    }
    if mode == 4 || mode == 5 || mode == 6 {
        assert!(keyboard.complete);
    }
    if mode == 3 || mode == 7 {
        assert!(touch.complete);
    }
    // SAFETY: validate and restore only this fixture's lifecycle observer.
    unsafe {
        acceptance_lifecycle_finish();
    }
    drop(game);
    if let Some(audio) = audio {
        audio.finish();
    }
    assert_eq!(drops.get(), 1);
    drop(image);
    drop(window);
    assert!(weak.upgrade().is_none());
    assert_eq!(snapshot.bytes(), &[0, 13, 10, 255]);
    // SAFETY: the fixture logs this static string and retains nothing.
    unsafe {
        acceptance_log(c"Rust APK assets passed".as_ptr());
    }
    // SAFETY: the C fixture logs this static string and retains no pointer.
    unsafe {
        acceptance_log(c"Rust scheduler and cleanup passed".as_ptr());
    }
}
