// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{
    Error, Game,
    camera::{Camera, Follow, Selection, Viewport},
};
#[test]
fn identity_selection_close_and_lifetime() {
    let game = Game::new().unwrap();
    let other = Game::new().unwrap();
    let actor = game
        .spawn(Default::default(), |p| async move {
            loop {
                p.frame().await;
            }
        })
        .unwrap();
    let main = game.main_camera().unwrap();
    assert_eq!(main.id(), game.main_camera().unwrap().id());
    main.set_position(10.0, 20.0).unwrap();
    assert_eq!(actor.resolve_camera().unwrap().position(), (10.0, 20.0));
    assert_eq!(main.close(), Err(Error::InvalidArgument));
    let camera = game
        .create_camera(Camera::new(3.0, 4.0, None).unwrap())
        .unwrap();
    let alias = camera.clone();
    let snapshot = camera.snapshot().unwrap();
    alias.move_by(1.0, 2.0).unwrap();
    assert_eq!(camera.snapshot().unwrap().position(), (4.0, 6.0));
    assert_eq!(snapshot.position(), (3.0, 4.0));
    actor
        .set_camera(Selection::Explicit(camera.clone()))
        .unwrap();
    assert_eq!(actor.resolve_camera().unwrap().position(), (4.0, 6.0));
    let foreign = other.main_camera().unwrap();
    assert_eq!(
        actor.set_camera(Selection::Explicit(foreign)),
        Err(Error::ForeignContext)
    );
    assert_eq!(actor.resolve_camera().unwrap().position(), (4.0, 6.0));
    camera.close().unwrap();
    alias.close().unwrap();
    assert!(matches!(camera.snapshot(), Err(Error::NotFound)));
    assert!(matches!(actor.resolve_camera(), Err(Error::NotFound)));
    assert!(
        matches!(actor.camera_selection().unwrap(),Selection::Explicit(ref c) if c.id() == camera.id())
    );
    assert_eq!(
        actor.set_camera(Selection::Explicit(camera.clone())),
        Err(Error::NotFound)
    );
    actor.set_camera(Selection::Screen).unwrap();
    assert!(matches!(
        actor.camera_selection().unwrap(),
        Selection::Screen
    ));
    assert_eq!(actor.resolve_camera().unwrap().position(), (0.0, 0.0));
    actor.set_camera(Selection::Main).unwrap();
    assert!(matches!(actor.camera_selection().unwrap(), Selection::Main));
    let id = game
        .create_camera(Camera::new(5.0, 6.0, None).unwrap())
        .unwrap()
        .id();
    assert!(id > camera.id());
    for _ in 0..256 {
        let temporary = game.create_camera(snapshot).unwrap();
        temporary.close().unwrap();
    }
    for _ in 0..16 {
        let _ = game.create_camera(snapshot).unwrap();
    }
    actor.kill().unwrap();
    assert_eq!(actor.set_camera(Selection::Main), Err(Error::NotFound));
    drop(game);
    assert!(matches!(main.snapshot(), Err(Error::Closed)));
    assert_eq!(main.close(), Err(Error::Closed));
    assert!(matches!(actor.camera_selection(), Err(Error::Closed)));
}
#[test]
fn registered_follow_and_error_atomicity() {
    let mut game = Game::new().unwrap();
    let other = Game::new().unwrap();
    let actor = game
        .spawn(Default::default(), |p| async move {
            loop {
                p.frame().await;
            }
        })
        .unwrap();
    actor.set_position(100.0, 80.0).unwrap();
    let camera = game
        .create_camera(
            Camera::new(
                3.0,
                4.0,
                Some(Viewport {
                    x: 10,
                    y: 20,
                    width: 40,
                    height: 60,
                }),
            )
            .unwrap(),
        )
        .unwrap();
    let mut follow = Follow::new(&actor).unwrap();
    game.tick().unwrap();
    assert!(follow.update_handle(&camera, (100, 80)).unwrap());
    assert_eq!(camera.snapshot().unwrap().position(), (80.0, 50.0));
    let before = camera.snapshot().unwrap().position();
    assert_eq!(
        camera.set_position(f64::NAN, 0.0),
        Err(Error::InvalidArgument)
    );
    assert_eq!(
        follow.update_handle(&other.main_camera().unwrap(), (100, 80)),
        Err(Error::ForeignContext)
    );
    assert_eq!(
        follow.update_handle(&camera, (0, 80)),
        Err(Error::InvalidArgument)
    );
    assert_eq!(camera.snapshot().unwrap().position(), before);
    actor.kill().unwrap();
    game.tick().unwrap();
    assert!(!follow.update_handle(&camera, (100, 80)).unwrap());
    assert_eq!(camera.snapshot().unwrap().position(), before);
    camera.close().unwrap();
    assert_eq!(
        follow.update_handle(&camera, (100, 80)),
        Err(Error::NotFound)
    );
}
