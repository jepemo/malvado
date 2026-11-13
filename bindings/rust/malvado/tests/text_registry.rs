// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{Error, Game, graphic::TextAlignment};
#[test]
fn context_and_process_lifetimes() {
    let mut game = Game::new().unwrap();
    let host = game
        .create_text("host", (i32::MIN, i32::MAX), TextAlignment::Center)
        .unwrap();
    let saved = host.snapshot().unwrap();
    let copy = host.clone();
    drop(host);
    copy.update("updated", (1, 2), TextAlignment::Right)
        .unwrap();
    assert_eq!(saved.value, "host");
    assert_eq!(copy.snapshot().unwrap().value, "updated");
    assert!(matches!(
        copy.update("bad\0", (0, 0), TextAlignment::Left),
        Err(Error::InvalidArgument)
    ));
    assert_eq!(copy.snapshot().unwrap().position, (1, 2));
    let actor = game
        .spawn(Default::default(), |p| async move {
            loop {
                p.frame().await;
            }
        })
        .unwrap();
    let owned = actor
        .create_text("owned", (0, 0), TextAlignment::Left)
        .unwrap();
    actor.kill().unwrap();
    assert_eq!(owned.snapshot().unwrap().value, "owned");
    game.tick().unwrap();
    assert!(matches!(owned.snapshot(), Err(Error::NotFound)));
    assert!(matches!(
        actor.create_text("dead", (0, 0), TextAlignment::Left),
        Err(Error::NotFound)
    ));
    let done = game
        .spawn(Default::default(), |_| async { Ok(()) })
        .unwrap();
    let returned = done
        .create_text("return", (0, 0), TextAlignment::Left)
        .unwrap();
    game.tick().unwrap();
    assert!(matches!(returned.remove(), Err(Error::NotFound)));
    copy.remove().unwrap();
    assert!(matches!(copy.remove(), Err(Error::NotFound)));
    let closed = game
        .create_text("shutdown", (0, 0), TextAlignment::Left)
        .unwrap();
    drop(game);
    assert!(matches!(closed.snapshot(), Err(Error::Closed)));
    assert!(matches!(closed.remove(), Err(Error::Closed)));
}
