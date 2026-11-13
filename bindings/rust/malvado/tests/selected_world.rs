// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{
    Error, Game, Signal,
    collision::{Rect, Shape, Transform, World},
    graphic::{Color, Geometry, Graphic, Pivot},
};
fn world(rect: Rect) -> World {
    let mut world = World::new().unwrap();
    world.add_rect(rect).unwrap();
    world
}
#[test]
fn graph_local_disabled_and_world_lifetime() {
    let game = Game::new().unwrap();
    let actor = game
        .spawn(Default::default(), |p| async move {
            loop {
                p.frame().await;
            }
        })
        .unwrap();
    let world = world(Rect {
        x: 10.0,
        y: -10.0,
        width: 1.0,
        height: 20.0,
    });
    actor
        .set_graphic(Graphic {
            geometry: Geometry::rectangle(4.0, 2.0, Pivot::Center).unwrap(),
            color: Color::default(),
        })
        .unwrap();
    assert_eq!(
        world.move_selected(&actor, 100.0, 0.0).unwrap().hit,
        Some(1)
    );
    assert_eq!(actor.position().unwrap(), (8.0, 0.0));
    assert_eq!(world.query_selected(&actor).unwrap(), None);
    world.move_selected(&actor, 0.0, 1.0).unwrap();
    assert_eq!(actor.position().unwrap(), (8.0, 1.0));
    actor.set_position(0.0, 0.0).unwrap();
    actor.set_angle(90.0).unwrap();
    actor.set_scale(2.0).unwrap();
    assert_eq!(
        world.move_selected(&actor, 100.0, 0.0).unwrap().hit,
        Some(1)
    );
    assert!((actor.position().unwrap().0 - 8.0).abs() < 1e-12);
    actor.set_position(0.0, 0.0).unwrap();
    actor
        .bind_collider(Some(Shape::circle(0.5).unwrap()))
        .unwrap();
    actor.clear_graphic().unwrap();
    world.move_selected(&actor, 100.0, 0.0).unwrap();
    assert_eq!(actor.position().unwrap(), (9.0, 0.0));
    actor.clear_collider().unwrap();
    actor.set_scale(0.0).unwrap();
    game.signal(&actor, Signal::Freeze).unwrap();
    assert_eq!(world.move_selected(&actor, 100.0, 0.0).unwrap().hit, None);
    assert_eq!(actor.position().unwrap(), (109.0, 0.0));
    assert_eq!(world.query_selected(&actor).unwrap(), None);
    actor.bind_collider(None).unwrap(); // No graph: free even with zero scale.
    assert_eq!(world.move_selected(&actor, 1.0, 0.0).unwrap().hit, None);
    actor.kill().unwrap();
    assert_eq!(world.query_selected(&actor), Err(Error::NotFound));
    assert_eq!(world.move_selected(&actor, 1.0, 0.0), Err(Error::NotFound));
    drop(game);
    assert_eq!(world.query_selected(&actor), Err(Error::Closed));
    assert_eq!(world.move_selected(&actor, 1.0, 0.0), Err(Error::Closed));
}
#[test]
fn fixed_offset_translation_and_failure_atomicity() {
    let game = Game::new().unwrap();
    let actor = game
        .spawn(Default::default(), |p| async move {
            loop {
                p.frame().await;
            }
        })
        .unwrap();
    let wall = world(Rect {
        x: 10.0,
        y: -10.0,
        width: 1.0,
        height: 20.0,
    });
    let probe = world(Rect {
        x: 8.0,
        y: -1.0,
        width: 2.0,
        height: 3.0,
    });
    let shape = Shape::circle(1.0).unwrap();
    actor.set_collider(shape, Transform::default()).unwrap();
    actor.set_position(100.0, 0.0).unwrap();
    actor.set_scale(0.0).unwrap();
    assert_eq!(wall.move_selected(&actor, 100.0, 0.0).unwrap().hit, Some(1));
    assert_eq!(actor.position().unwrap(), (109.0, 0.0));
    assert_eq!(probe.query_selected(&actor).unwrap(), Some(1));
    actor.set_position(500.0, 0.0).unwrap();
    assert_eq!(probe.query_selected(&actor).unwrap(), Some(1));
    wall.move_selected(&actor, 0.0, 1.0).unwrap();
    assert_eq!(actor.position().unwrap(), (500.0, 1.0));
    for value in [f64::NAN, f64::INFINITY, f64::NEG_INFINITY, f64::MAX] {
        assert_eq!(
            wall.move_selected(&actor, value, 0.0),
            Err(Error::InvalidArgument)
        );
        assert_eq!(
            wall.move_selected(&actor, 0.0, value),
            Err(Error::InvalidArgument)
        );
    }
    assert_eq!(actor.position().unwrap(), (500.0, 1.0));
    assert_eq!(probe.query_selected(&actor).unwrap(), Some(1));
    actor.set_position(f64::MAX, 0.0).unwrap();
    let empty = World::new().unwrap();
    assert_eq!(
        empty.move_selected(&actor, f64::MAX / 16.0, 0.0),
        Err(Error::InvalidArgument)
    );
    assert_eq!(actor.position().unwrap(), (f64::MAX, 0.0));
    assert_eq!(probe.query_selected(&actor).unwrap(), Some(1));
    actor.set_position(0.0, 0.0).unwrap();
    actor.set_scale(1.0).unwrap();
    actor.bind_collider(Some(shape)).unwrap();
    actor.set_scale(0.0).unwrap();
    assert_eq!(wall.query_selected(&actor), Err(Error::InvalidArgument));
    assert_eq!(
        wall.move_selected(&actor, 1.0, 0.0),
        Err(Error::InvalidArgument)
    );
    assert_eq!(actor.position().unwrap(), (0.0, 0.0));
    actor.set_scale(1.0).unwrap();
    actor.set_position(10.0, 0.0).unwrap();
    assert_eq!(wall.query_selected(&actor).unwrap(), Some(1));
    assert_eq!(
        wall.move_selected(&actor, 1.0, 0.0),
        Err(Error::InvalidArgument)
    );
    assert_eq!(actor.position().unwrap(), (10.0, 0.0));
}
