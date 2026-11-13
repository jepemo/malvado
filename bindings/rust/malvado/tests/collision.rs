// Malvado 2; SPDX-License-Identifier: Apache-2.0
#![cfg(not(miri))]
use malvado::collision::{Rect, Shape, Transform, World};
use malvado::{Error, Game, Relation, Signal, SpawnOptions};
use std::future::pending;

fn wall() -> Rect {
    Rect {
        x: 10.0,
        y: 0.0,
        width: 10.0,
        height: 10.0,
    }
}
fn start() -> Transform {
    Transform {
        y: 5.0,
        ..Default::default()
    }
}

#[test]
fn geometry_validation_offsets_rotation_and_tangency_use_native_math() {
    for radius in [0.0, -1.0, f64::NAN, f64::INFINITY] {
        assert!(matches!(Shape::circle(radius), Err(Error::InvalidArgument)));
    }
    assert!(Shape::rectangle(0.0, 1.0).is_err());
    let circle = Shape::circle(1.0).unwrap();
    assert!(circle.with_offset(f64::NAN, 0.0).is_err());
    let identity = Transform::default();
    assert!(
        !circle
            .overlaps(identity, circle, Transform { x: 2.0, ..identity })
            .unwrap()
    );
    let offset = circle.with_offset(2.0, 0.0).unwrap();
    assert!(
        offset
            .overlaps(
                Transform {
                    angle: 90.0,
                    ..identity
                },
                circle,
                Transform { y: 2.0, ..identity }
            )
            .unwrap()
    );
    assert_eq!(
        circle.overlaps(
            identity,
            circle,
            Transform {
                scale: 0.0,
                ..identity
            }
        ),
        Err(Error::InvalidArgument)
    );
    let rect = wall();
    assert!(rect.contains(10.0, 0.0).unwrap());
    assert!(!rect.contains(20.0, 0.0).unwrap());
    assert!(!rect.overlaps(Rect { x: 20.0, ..rect }).unwrap());
    assert_eq!(
        Rect { width: 0.0, ..rect }.overlaps(rect),
        Err(Error::InvalidArgument)
    );
}

#[test]
fn fast_motion_contact_and_sliding_match_the_accepted_world_example() {
    let mut world = World::new().unwrap();
    assert_eq!(world.add_rect(wall()).unwrap(), 1);
    let shape = Shape::circle(1.0).unwrap();
    let mut position = start();
    let sweep = world.sweep(shape, position, 100.0, 0.0).unwrap();
    assert_eq!(sweep.hit, Some(1));
    assert!((sweep.fraction - 0.09).abs() < 1e-12);
    assert_eq!((sweep.normal_x, sweep.normal_y), (-1.0, 0.0));
    assert_eq!(
        world.move_by(shape, &mut position, 100.0, 0.0).unwrap(),
        sweep
    );
    assert_eq!((position.x, position.y), (9.0, 5.0));
    assert_eq!(world.query(shape, position).unwrap(), None);
    assert_eq!(
        world.move_by(shape, &mut position, 1.0, 0.0).unwrap().hit,
        Some(1)
    );
    assert_eq!(position.x, 9.0);
    assert_eq!(
        world.move_by(shape, &mut position, 0.0, 1.0).unwrap().hit,
        None
    );
    assert_eq!((position.x, position.y), (9.0, 6.0));
    let rotated = Shape::rectangle(2.0, 2.0).unwrap();
    let mut position = Transform {
        angle: 45.0,
        ..start()
    };
    assert_eq!(
        world
            .move_by(rotated, &mut position, 100.0, 0.0)
            .unwrap()
            .hit,
        Some(1)
    );
    assert!(position.x < 9.0);
    assert_eq!(world.query(rotated, position).unwrap(), None);
}

#[test]
fn failed_motion_preserves_input_and_failed_insert_preserves_ids() {
    let mut world = World::new().unwrap();
    assert_eq!(
        world.add_rect(Rect {
            width: f64::NAN,
            ..wall()
        }),
        Err(Error::WorldInsert)
    );
    assert_eq!(world.add_rect(wall()).unwrap(), 1);
    let shape = Shape::circle(1.0).unwrap();
    for (x, dx) in [(11.0, 1.0), (0.0, f64::NAN), (0.0, 1e100)] {
        let mut position = Transform { x, ..start() };
        let original = position;
        assert_eq!(
            world.move_by(shape, &mut position, dx, 0.0),
            Err(Error::InvalidArgument)
        );
        assert_eq!(position, original);
    }
    assert_eq!(world.add_rect(Rect { x: 50.0, ..wall() }).unwrap(), 2);
}

#[test]
fn corner_circle_collision_does_not_leave_an_overlap() {
    let mut world = World::new().unwrap();
    world.add_rect(wall()).unwrap();
    let shape = Shape::circle(1.0).unwrap();
    let mut position = Transform {
        y: 10.5,
        ..Default::default()
    };
    let sweep = world.move_by(shape, &mut position, 100.0, 0.0).unwrap();
    assert_eq!(sweep.hit, Some(1));
    assert!(sweep.normal_x < 0.0 && sweep.normal_y > 0.0);
    assert_eq!(world.query(shape, position).unwrap(), None);
}

#[test]
fn process_colliders_support_types_identity_sleep_freeze_and_clear() {
    let mut game = Game::new().unwrap();
    let source = game
        .spawn(SpawnOptions::default(), |_| async {
            pending::<()>().await;
            Ok(())
        })
        .unwrap();
    let mut targets = Vec::new();
    let shape = Shape::circle(2.0).unwrap();
    source.set_collider(shape, Transform::default()).unwrap();
    for index in 1..=3 {
        let target = game
            .spawn(
                SpawnOptions {
                    type_id: index % 2,
                    ..Default::default()
                },
                |_| async {
                    pending::<()>().await;
                    Ok(())
                },
            )
            .unwrap();
        target
            .set_collider(
                shape,
                Transform {
                    x: index as f64,
                    ..Default::default()
                },
            )
            .unwrap();
        targets.push(target);
    }
    assert!(!source.collides_with(&source).unwrap());
    assert!(source.collides_with(&targets[0]).unwrap());
    let all = source.collisions(0).unwrap();
    assert!(all.iter().zip(&targets).all(|(a, b)| a == b));
    assert_eq!(all.len(), 3);
    assert_eq!(source.collisions(1).unwrap().len(), 2);
    assert!(source.first_collision(0).unwrap().as_ref().unwrap() == &targets[0]);
    game.signal(&targets[0], Signal::Sleep).unwrap();
    game.signal(&targets[1], Signal::Freeze).unwrap();
    assert_eq!(source.collisions(0).unwrap().len(), 3);
    assert_eq!(
        source.set_collider(
            shape,
            Transform {
                scale: 0.0,
                ..Default::default()
            }
        ),
        Err(Error::InvalidArgument)
    );
    assert!(source.collides_with(&targets[0]).unwrap());
    targets[0].clear_collider().unwrap();
    targets[0].clear_collider().unwrap();
    assert_eq!(source.collisions(0).unwrap().len(), 2);
    game.signal_tree(&targets[1], Signal::Kill, Relation::Ownership)
        .unwrap();
    game.tick().unwrap();
    assert_eq!(
        targets[1].set_collider(shape, Transform::default()),
        Err(Error::NotFound)
    );
    drop(game);
    assert_eq!(source.clear_collider(), Err(Error::Closed));
}

#[test]
fn foreign_colliders_are_rejected_and_world_data_can_outlive_context() {
    let game = Game::new().unwrap();
    let other = Game::new().unwrap();
    let a = game
        .spawn(SpawnOptions::default(), |_| async { Ok(()) })
        .unwrap();
    let b = other
        .spawn(SpawnOptions::default(), |_| async { Ok(()) })
        .unwrap();
    assert_eq!(a.collides_with(&b), Err(Error::ForeignContext));
    let mut world = World::new().unwrap();
    world.add_rect(wall()).unwrap();
    let retained = game.resource(world).unwrap();
    drop(game);
    assert_eq!(
        retained
            .query(Shape::circle(1.0).unwrap(), start())
            .unwrap(),
        None
    );
    drop(retained);
}

#[test]
fn no_collider_returns_no_hit_and_stale_sources_are_errors() {
    let mut game = Game::new().unwrap();
    let source = game
        .spawn(SpawnOptions::default(), |_| async { Ok(()) })
        .unwrap();
    assert!(source.first_collision(0).unwrap().is_none());
    assert!(source.collisions(0).unwrap().is_empty());
    game.tick().unwrap();
    assert!(matches!(source.collisions(0), Err(Error::NotFound)));
}

#[test]
fn repeated_world_allocation_and_cleanup_is_independent_of_the_scheduler() {
    for _ in 0..256 {
        let mut world = World::new().unwrap();
        for index in 0..8 {
            world
                .add_rect(Rect {
                    x: 30.0 * index as f64,
                    ..wall()
                })
                .unwrap();
        }
        assert_eq!(
            world
                .query(
                    Shape::circle(1.0).unwrap(),
                    Transform {
                        x: 15.0,
                        y: 5.0,
                        ..Default::default()
                    }
                )
                .unwrap(),
            None
        );
    }
}

#[test]
fn world_trace_matches_c_and_lua_process_movement() {
    let mut world = World::new().unwrap();
    world.add_rect(wall()).unwrap();
    let shape = Shape::circle(1.0).unwrap();
    let game = Game::new().unwrap();
    let actor = game
        .spawn(Default::default(), |p| async move {
            loop {
                p.frame().await;
            }
        })
        .unwrap();
    actor.set_position(0.0, 5.0).unwrap();
    actor.bind_collider(Some(shape)).unwrap();
    let mut position = start();
    let sweep = world.sweep(shape, position, 100.0, 0.0).unwrap();
    assert_eq!(sweep.hit, Some(1));
    assert_eq!((sweep.normal_x, sweep.normal_y), (-1.0, 0.0));
    let mut trace = format!("sweep hit=1 fraction={:.6} normal=-1,0\n", sweep.fraction);
    assert_eq!(
        world.move_selected(&actor, 100.0, 0.0).unwrap().hit,
        Some(1)
    );
    (position.x, position.y) = actor.position().unwrap();
    assert_eq!(world.query_selected(&actor).unwrap(), None);
    trace.push_str(&format!(
        "move x={:.6} y={:.6} clear=1\n",
        position.x, position.y
    ));
    assert_eq!(world.move_selected(&actor, 0.0, 1.0).unwrap().hit, None);
    (position.x, position.y) = actor.position().unwrap();
    trace.push_str(&format!("slide x={:.6} y={:.6}\n", position.x, position.y));
    actor.set_position(11.0, 6.0).unwrap();
    let original = actor.position().unwrap();
    assert_eq!(
        world.move_selected(&actor, 1.0, 0.0),
        Err(Error::InvalidArgument)
    );
    assert_eq!(actor.position().unwrap(), original);
    trace.push_str("failure unchanged=1\n");
    assert_eq!(trace, include_str!("fixtures/world.txt"));
}

#[test]
fn contextual_world_movement_properties_errors_and_lifetime() {
    let mut game = Game::new().unwrap();
    let actor = game
        .spawn(Default::default(), |p| async move {
            loop {
                p.frame().await;
            }
        })
        .unwrap();
    let mut world = World::new().unwrap();
    world
        .add_rect(Rect {
            x: 10.0,
            y: -10.0,
            width: 10.0,
            height: 20.0,
        })
        .unwrap();
    let shape = Shape::rectangle(4.0, 2.0).unwrap();
    actor.set_angle(90.0).unwrap();
    actor.set_scale(2.0).unwrap();
    assert_eq!(
        world
            .move_process(&actor, Some(shape), 100.0, 0.0)
            .unwrap()
            .hit,
        Some(1)
    );
    let contact = actor.position().unwrap();
    assert!((contact.0 - 8.0).abs() < 1e-12);
    assert_eq!(actor.angle().unwrap(), 90.0);
    assert_eq!(actor.scale().unwrap(), 2.0);
    assert_eq!(world.query_process(&actor, Some(shape)).unwrap(), None);
    for signal in [Signal::Sleep, Signal::Freeze] {
        game.signal(&actor, signal).unwrap();
        world.move_process(&actor, None, 0.0, 1.0).unwrap();
        game.tick().unwrap();
    }
    assert_eq!(actor.position().unwrap().1, 2.0);
    for value in [f64::NAN, f64::INFINITY, f64::NEG_INFINITY, f64::MAX] {
        let before = actor.position().unwrap();
        assert_eq!(
            world.move_process(&actor, None, value, 0.0),
            Err(Error::InvalidArgument)
        );
        assert_eq!(
            world.move_process(&actor, None, 0.0, value),
            Err(Error::InvalidArgument)
        );
        assert_eq!(actor.position().unwrap(), before);
    }
    actor.set_position(11.0, 0.0).unwrap();
    assert_eq!(world.query_process(&actor, Some(shape)).unwrap(), Some(1));
    assert_eq!(
        world.move_process(&actor, Some(shape), 1.0, 0.0),
        Err(Error::InvalidArgument)
    );
    assert_eq!(actor.position().unwrap(), (11.0, 0.0));
    actor.set_scale(0.0).unwrap();
    assert_eq!(
        world.query_process(&actor, Some(shape)),
        Err(Error::InvalidArgument)
    );
    assert_eq!(
        world.move_process(&actor, Some(shape), 1.0, 0.0),
        Err(Error::InvalidArgument)
    );
    assert_eq!(world.query_process(&actor, None).unwrap(), None);
    assert_eq!(
        world.move_process(&actor, None, 100.0, 0.0).unwrap().hit,
        None
    );
    assert_eq!(actor.position().unwrap(), (111.0, 0.0));
    actor.set_position(f64::MAX, 0.0).unwrap();
    assert_eq!(
        world.move_process(&actor, None, f64::MAX / 16.0, 0.0),
        Err(Error::InvalidArgument)
    );
    assert_eq!(actor.position().unwrap(), (f64::MAX, 0.0));
    actor.kill().unwrap();
    assert_eq!(world.query_process(&actor, None), Err(Error::NotFound));
    assert_eq!(
        world.move_process(&actor, None, 1.0, 0.0),
        Err(Error::NotFound)
    );
    drop(game);
    assert_eq!(world.query_process(&actor, None), Err(Error::Closed));
    assert_eq!(
        world.move_process(&actor, None, 1.0, 0.0),
        Err(Error::Closed)
    );
}
