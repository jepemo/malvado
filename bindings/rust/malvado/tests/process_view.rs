// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{
    Error, Game, Signal,
    camera::{Camera, Selection, Viewport},
};

#[test]
fn copied_descriptors_composition_errors_and_lifetime() {
    let mut game = Game::new().unwrap();
    let actor = game
        .spawn(Default::default(), |p| async move {
            loop {
                p.frame().await;
            }
        })
        .unwrap();
    assert_eq!(actor.region().unwrap(), None);
    assert_eq!(actor.scroll().unwrap(), (0.0, 0.0));
    actor.set_position(60.0, 50.0).unwrap();
    game.signal(&actor, Signal::Freeze).unwrap();
    let alias = actor.clone();
    let mut region = Viewport {
        x: 10,
        y: 20,
        width: 40,
        height: 30,
    };
    actor.set_region(Some(region)).unwrap();
    region.width = 1;
    assert_eq!(alias.region().unwrap().unwrap().width, 40);
    actor.set_scroll((3.0, 4.0)).unwrap();
    game.main_camera()
        .unwrap()
        .set_position(100.0, 50.0)
        .unwrap();
    game.tick().unwrap();
    assert_eq!(alias.view_offset().unwrap(), (-103.0, -54.0));
    assert_eq!(actor.position().unwrap(), (60.0, 50.0));
    let host = Viewport {
        x: 20,
        y: 25,
        width: 10,
        height: 10,
    };
    assert_eq!(actor.view_clip(Some(host), (100, 80)).unwrap(), Some(host));
    assert_eq!(
        actor.set_region(Some(Viewport { width: 0, ..region })),
        Err(Error::InvalidArgument)
    );
    assert_eq!(actor.region().unwrap().unwrap().width, 40);
    for value in [f32::NAN, f32::INFINITY, -f32::INFINITY, f32::MAX] {
        assert_eq!(actor.set_scroll((value, 0.0)), Err(Error::InvalidArgument));
        assert_eq!(actor.set_scroll((0.0, value)), Err(Error::InvalidArgument));
        assert_eq!(actor.scroll().unwrap(), (3.0, 4.0));
    }
    assert_eq!(actor.view_clip(None, (0, 80)), Err(Error::InvalidArgument));
    actor
        .set_region(Some(Viewport {
            x: i32::MAX,
            y: i32::MIN,
            width: i32::MAX,
            height: i32::MAX,
        }))
        .unwrap();
    assert_eq!(actor.view_clip(None, (100, 80)).unwrap(), None);
    assert_eq!(
        actor.view_clip(Some(Viewport { width: 0, ..host }), (100, 80)),
        Err(Error::InvalidArgument)
    );
    actor.set_region(None).unwrap();
    let camera = game
        .create_camera(Camera::new(1.0, 2.0, Some(host)).unwrap())
        .unwrap();
    actor
        .set_camera(Selection::Explicit(camera.clone()))
        .unwrap();
    assert_eq!(actor.view_offset().unwrap(), (16.0, 19.0));
    assert_eq!(actor.view_clip(None, (100, 80)).unwrap(), Some(host));
    camera.close().unwrap();
    assert_eq!(actor.view_offset(), Err(Error::NotFound));
    assert_eq!(actor.view_clip(None, (100, 80)), Err(Error::NotFound));
    // Descriptors remain readable and editable while the selected view is closed.
    actor.set_scroll((0.0, 0.0)).unwrap();
    actor.set_camera(Selection::Screen).unwrap();
    assert_eq!(actor.view_offset().unwrap(), (0.0, 0.0));
    assert_eq!(
        actor.view_clip(None, (100, 80)).unwrap(),
        Some(Viewport {
            x: 0,
            y: 0,
            width: 100,
            height: 80
        })
    );
    actor.kill().unwrap();
    assert_eq!(actor.region(), Err(Error::NotFound));
    assert_eq!(actor.set_region(None), Err(Error::NotFound));
    assert_eq!(actor.scroll(), Err(Error::NotFound));
    assert_eq!(actor.set_scroll((0.0, 0.0)), Err(Error::NotFound));
    drop(game);
    assert_eq!(alias.region(), Err(Error::Closed));
    assert_eq!(alias.view_offset(), Err(Error::Closed));
    assert_eq!(alias.view_clip(None, (100, 80)), Err(Error::Closed));
}
