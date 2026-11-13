// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{
    Error, Game, Handle, Signal,
    collision::{Rect, Shape, Transform, World},
    graphic::{Color, Geometry, Graphic, Pivot},
};
fn actor(game: &Game, geometry: Geometry, x: f64) -> Handle {
    let handle = game
        .spawn(Default::default(), |p| async move {
            loop {
                p.frame().await;
            }
        })
        .unwrap();
    handle.set_type(1).unwrap();
    handle
        .set_graphic(Graphic {
            geometry,
            color: Color {
                r: 255,
                g: 255,
                b: 255,
                a: 255,
            },
        })
        .unwrap();
    handle.set_position(x, 0.0).unwrap();
    handle
}
fn trace(source: &Handle, phase: &str, out: &mut String) {
    let hits = source.collisions(1).unwrap();
    assert!(hits.len() <= 1);
    let first = source.first_collision(1).unwrap();
    assert_eq!(first.map(|h| h.id()), hits.first().map(|h| h.id()));
    out.push_str(&format!(
        "{phase} hits={}\n",
        hits.first().map_or("-".to_owned(), |h| h.id().to_string())
    ));
}
#[test]
fn bound_trace_matches_c_and_lua() {
    let game = Game::new().unwrap();
    let rect = Geometry::rectangle(4.0, 2.0, Pivot::Center).unwrap();
    let circle = Geometry::circle(1.0).unwrap();
    let a = actor(&game, rect, 0.0);
    let b = actor(&game, circle, 2.0);
    let c = actor(&game, circle, 7.75);
    let mut out = String::new();
    trace(&a, "initial", &mut out);
    a.set_position(10.0, 0.0).unwrap();
    trace(&a, "position", &mut out);
    a.set_angle(90.0).unwrap();
    trace(&a, "rotation", &mut out);
    a.set_scale(2.0).unwrap();
    trace(&a, "scale", &mut out);
    a.bind_collider(Some(Shape::circle(0.25).unwrap())).unwrap();
    trace(&a, "override", &mut out);
    a.clear_collider().unwrap();
    trace(&a, "disabled", &mut out);
    a.bind_collider(None).unwrap();
    trace(&a, "restored", &mut out);
    a.clear_graphic().unwrap();
    trace(&a, "cleared", &mut out);
    a.set_graphic(Graphic {
        geometry: rect,
        color: Color::default(),
    })
    .unwrap();
    trace(&a, "reassigned", &mut out);
    a.set_position(0.0, 0.0).unwrap();
    a.set_angle(0.0).unwrap();
    a.set_scale(1.0).unwrap();
    let mut world = World::new().unwrap();
    world
        .add_rect(Rect {
            x: 10.0,
            y: -10.0,
            width: 1.0,
            height: 20.0,
        })
        .unwrap();
    assert!(
        world
            .move_process(&a, Some(rect.collider()), 100.0, 0.0)
            .unwrap()
            .hit
            .is_some()
    );
    trace(&a, "world", &mut out);
    game.signal(&b, Signal::Sleep).unwrap();
    game.signal(&c, Signal::Freeze).unwrap();
    trace(&a, "inactive", &mut out);
    c.set_scale(0.0).unwrap();
    trace(&a, "invalid-target", &mut out);
    assert_eq!(a.collides_with(&c), Err(Error::InvalidArgument));
    assert_eq!(out, include_str!("fixtures/bound_collider.txt"));
}
#[test]
fn bound_errors_fixed_override_recovery_and_lifetime() {
    let game = Game::new().unwrap();
    let geometry = Geometry::circle(1.0).unwrap();
    let a = actor(&game, geometry, 0.0);
    let b = actor(&game, geometry, 0.0);
    assert!(a.collides_with(&b).unwrap());
    a.set_collider(Shape::circle(1.0).unwrap(), Transform::default())
        .unwrap();
    a.set_position(100.0, 0.0).unwrap();
    a.set_scale(0.0).unwrap();
    assert_eq!(a.bind_collider(None), Err(Error::InvalidArgument));
    assert!(a.collides_with(&b).unwrap());
    a.clear_collider().unwrap();
    assert!(!a.collides_with(&b).unwrap());
    a.set_scale(1.0).unwrap();
    a.bind_collider(None).unwrap();
    assert!(!a.collides_with(&b).unwrap());
    a.set_position(0.0, 0.0).unwrap();
    assert!(a.collides_with(&b).unwrap());
    a.set_scale(0.0).unwrap();
    assert!(matches!(a.collisions(1), Err(Error::InvalidArgument)));
    assert!(matches!(a.first_collision(1), Err(Error::InvalidArgument)));
    a.set_scale(1.0).unwrap();
    assert!(a.collides_with(&b).unwrap());
    let local = Shape::circle(0.5).unwrap().with_offset(2.0, 0.0).unwrap();
    a.bind_collider(Some(local)).unwrap();
    b.set_position(0.0, 3.0).unwrap();
    a.set_angle(90.0).unwrap();
    assert!(a.collides_with(&b).unwrap());
    a.set_angle(0.0).unwrap();
    assert!(!a.collides_with(&b).unwrap());
    a.clear_graphic().unwrap();
    a.set_angle(90.0).unwrap();
    assert!(a.collides_with(&b).unwrap()); // Local override survives graph clearing.
    a.bind_collider(None).unwrap();
    assert!(!a.collides_with(&b).unwrap());
    a.kill().unwrap();
    assert_eq!(a.bind_collider(None), Err(Error::NotFound));
    drop(game);
    assert_eq!(a.bind_collider(None), Err(Error::Closed));
}
