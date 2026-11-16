// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{Error, Game, SpawnOptions, asset::Source, image::Image, window::Window};
use std::{cell::Cell, rc::Rc};
pub fn process(window: Rc<Window>, snapshot: Source, image: Image) {
    let weak = Rc::downgrade(&window);
    let count = Rc::new(Cell::new(0));
    let guard = super::Guard(count.clone());
    let mut game = Game::new().unwrap();
    game.spawn(SpawnOptions::default(), move |process| async move {
        let _guard = guard;
        let _image = process.resource(image)?;
        Err(Error::Process)
    })
    .unwrap();
    assert_eq!(game.tick(), Err(Error::Process));
    assert_eq!(count.get(), 1);
    assert_eq!(game.process_count(), 0);
    // SAFETY: the fixture logs a static string and retains nothing.
    unsafe {
        super::acceptance_log(c"Rust process error observed".as_ptr());
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
