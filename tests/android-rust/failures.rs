// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{Error, Game, asset::Source, image::Image, window::Window};
use std::{cell::Cell, rc::Rc};
pub fn run(window: Rc<Window>, snapshot: Source, image: Result<Image, Error>, failure: i32) {
    if failure == 2 {
        assert_eq!(
            image.as_ref().err(),
            Some(&Error::Platform("image load or creation failed"))
        );
    } else {
        assert_eq!(failure, 1);
        assert!(image.is_ok());
    }
    let weak = Rc::downgrade(&window);
    let count = Rc::new(Cell::new(0));
    let guard = super::Guard(count.clone());
    let mut game = Game::new().unwrap();
    game.spawn(move |process| async move {
        let _guard = guard;
        let _image = process.resource(image?)?;
        Err(Error::Process)
    })
    .unwrap();
    assert_eq!(game.tick(), Err(Error::Process));
    assert_eq!(count.get(), 1);
    assert_eq!(game.process_count(), 0);
    // SAFETY: the fixture logs a static string and retains nothing.
    unsafe {
        super::acceptance_log(if failure == 2 {
            c"Rust missing-image error observed".as_ptr()
        } else {
            c"Rust process error observed".as_ptr()
        });
    }
    drop(game);
    assert_eq!(count.get(), 1);
    drop(window);
    assert!(weak.upgrade().is_none());
    assert_eq!(snapshot.bytes(), &[0, 13, 10, 255]);
    // SAFETY: the fixture logs static strings and retains nothing.
    unsafe {
        super::acceptance_log(c"Rust scheduler and cleanup passed".as_ptr());
        super::acceptance_log(c"Rust failure resources closed".as_ptr());
    }
}
