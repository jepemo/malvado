// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{
    Engine, Error, Signal,
    graphic::{Color, Geometry, Graphic, Pivot},
};
#[test]
fn copied_graphic_assignment_and_handle_lifetimes() {
    let mut engine = Engine::new().unwrap();
    let actor = engine
        .spawn(Default::default(), |p| async move {
            loop {
                p.frame().await;
            }
        })
        .unwrap();
    assert!(actor.graphic().unwrap().is_none());
    let mut graphic = Graphic {
        geometry: Geometry::rectangle(4.0, 2.0, Pivot::TopLeft).unwrap(),
        color: Color {
            r: 1,
            g: 2,
            b: 3,
            a: 4,
        },
    };
    actor.set_graphic(graphic).unwrap();
    graphic.color.r = 99;
    assert_eq!(actor.graphic().unwrap().unwrap().color.r, 1);
    let mut snapshot = actor.graphic().unwrap().unwrap();
    snapshot.color.g = 99;
    assert_eq!(snapshot.color.g, 99);
    assert_eq!(actor.graphic().unwrap().unwrap().color.g, 2);
    for signal in [Signal::Sleep, Signal::Freeze, Signal::Wake] {
        engine.signal(&actor, signal).unwrap();
        engine.tick().unwrap();
        assert!(actor.graphic().unwrap().is_some());
    }
    let alias = actor.clone();
    alias
        .set_graphic(Graphic {
            geometry: Geometry::circle(3.0).unwrap(),
            ..graphic
        })
        .unwrap();
    assert_eq!(actor.graphic().unwrap().unwrap().color.r, 99);
    alias.clear_graphic().unwrap();
    actor.clear_graphic().unwrap();
    assert!(actor.graphic().unwrap().is_none());
    actor.kill().unwrap();
    assert!(matches!(actor.graphic(), Err(Error::NotFound)));
    assert_eq!(actor.set_graphic(graphic), Err(Error::NotFound));
    assert_eq!(actor.clear_graphic(), Err(Error::NotFound));
    drop(engine);
    assert!(matches!(alias.graphic(), Err(Error::Closed)));
    assert_eq!(alias.set_graphic(graphic), Err(Error::Closed));
    assert_eq!(alias.clear_graphic(), Err(Error::Closed));
}
