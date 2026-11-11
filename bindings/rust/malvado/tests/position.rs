// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{Engine, Error, Signal};

#[test]
fn atomic_position_and_handle_lifetime() {
    let mut engine = Engine::new().unwrap();
    let actor = engine
        .spawn(Default::default(), |p| async move {
            loop {
                p.frame().await;
            }
        })
        .unwrap();
    assert_eq!(actor.position().unwrap(), (0.0, 0.0));
    assert_eq!(actor.angle().unwrap(), 0.0);
    assert_eq!(actor.scale().unwrap(), 1.0);
    for value in [-f64::MAX, f64::MAX, -360.5, 0.0, 0.25] {
        actor.set_angle(value).unwrap();
        actor.set_scale(value).unwrap();
        assert_eq!(actor.angle().unwrap(), value);
        assert_eq!(actor.scale().unwrap(), value);
    }
    actor.set_position(-f64::MAX, f64::MAX).unwrap();
    for value in [f64::NAN, f64::INFINITY, f64::NEG_INFINITY] {
        assert_eq!(actor.set_position(value, 1.0), Err(Error::InvalidArgument));
        assert_eq!(actor.set_position(1.0, value), Err(Error::InvalidArgument));
        assert_eq!(actor.position().unwrap(), (-f64::MAX, f64::MAX));
        assert_eq!(actor.set_angle(value), Err(Error::InvalidArgument));
        assert_eq!(actor.set_scale(value), Err(Error::InvalidArgument));
        assert_eq!(actor.angle().unwrap(), 0.25);
        assert_eq!(actor.scale().unwrap(), 0.25);
    }
    actor.set_position(-1.5, 2.25).unwrap();
    let copied = actor.position().unwrap();
    let alias = actor.clone();
    alias.set_position(7.0, 8.0).unwrap();
    assert_eq!(copied, (-1.5, 2.25));
    assert_eq!(actor.position().unwrap(), (7.0, 8.0));
    for signal in [Signal::Sleep, Signal::Freeze, Signal::Wake] {
        engine.signal(&actor, signal).unwrap();
        engine.tick().unwrap();
        assert_eq!(actor.position().unwrap(), (7.0, 8.0));
    }
    actor.kill().unwrap();
    assert_eq!(actor.angle(), Err(Error::NotFound));
    assert_eq!(actor.scale(), Err(Error::NotFound));
    assert_eq!(actor.set_angle(0.0), Err(Error::NotFound));
    assert_eq!(actor.set_scale(1.0), Err(Error::NotFound));
    assert_eq!(actor.position(), Err(Error::NotFound));
    assert_eq!(actor.set_position(0.0, 0.0), Err(Error::NotFound));
    drop(engine);
    assert_eq!(alias.angle(), Err(Error::Closed));
    assert_eq!(alias.scale(), Err(Error::Closed));
    assert_eq!(alias.set_angle(0.0), Err(Error::Closed));
    assert_eq!(alias.set_scale(1.0), Err(Error::Closed));
    assert_eq!(alias.position(), Err(Error::Closed));
    assert_eq!(alias.set_position(0.0, 0.0), Err(Error::Closed));
}
