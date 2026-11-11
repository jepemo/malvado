// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{
    Error,
    camera::{Camera, Viewport},
};
#[test]
fn projection_center_and_atomic_errors() {
    let mut camera = Camera::new(100.0, 50.0, None).unwrap();
    camera.move_by(-10.0, 5.0).unwrap();
    assert_eq!(
        camera.project((100.0, 70.0), (3.0, 4.0)).unwrap(),
        (7.0, 11.0)
    );
    let before = camera.position();
    assert_eq!(
        camera.set_position(f64::NAN, 0.0),
        Err(Error::InvalidArgument)
    );
    assert_eq!(
        camera.move_by(f64::INFINITY, 0.0),
        Err(Error::InvalidArgument)
    );
    assert_eq!(
        camera.center((0.0, 0.0), (0, 80)),
        Err(Error::InvalidArgument)
    );
    assert_eq!(camera.position(), before);
    assert!(camera.project((f64::NAN, 0.0), (0.0, 0.0)).is_err());
    camera.center((100.0, 80.0), (100, 80)).unwrap();
    assert_eq!(camera.position(), (50.0, 40.0));
    assert!(Camera::new(f64::MAX, 0.0, None).is_err());
}
#[test]
fn viewport_clipping_and_copy_independence() {
    let viewport = Viewport {
        x: 40,
        y: 60,
        width: 300,
        height: 240,
    };
    let mut camera = Camera::new(100.0, 50.0, Some(viewport)).unwrap();
    let copy = camera;
    assert_eq!(
        camera.project((130.0, 80.0), (10.0, 5.0)).unwrap(),
        (60.0, 85.0)
    );
    camera.center((200.0, 150.0), (800, 600)).unwrap();
    assert_eq!(camera.position(), (50.0, 30.0));
    assert_eq!(copy.position(), (100.0, 50.0));
    let region = Viewport {
        x: 50,
        y: 70,
        width: 20,
        height: 30,
    };
    assert_eq!(camera.clip(Some(region), (640, 480)).unwrap(), Some(region));
    assert_eq!(
        camera
            .clip(
                Some(Viewport {
                    x: 0,
                    y: 0,
                    width: 10,
                    height: 10
                }),
                (640, 480)
            )
            .unwrap(),
        None
    );
    assert!(camera.clip(None, (0, 480)).is_err());
    assert!(
        camera
            .clip(Some(Viewport { width: 0, ..region }), (640, 480))
            .is_err()
    );
    assert!(
        Camera::new(
            0.0,
            0.0,
            Some(Viewport {
                height: 0,
                ..viewport
            })
        )
        .is_err()
    );
}

#[test]
fn native_follow_trace_matches_c_and_lua() {
    use malvado::{Engine, Signal, camera::Follow};
    let engine = Engine::new().unwrap();
    let actor = engine
        .spawn(Default::default(), |p| async move {
            loop {
                p.frame().await;
            }
        })
        .unwrap();
    actor.set_position(100.0, 80.0).unwrap();
    let mut follow = Follow::new(&actor).unwrap();
    let mut camera = Camera::new(3.0, 4.0, None).unwrap();
    assert!(follow.update(&mut camera, (100, 80)).unwrap());
    let mut trace = format!(
        "follow x={:.0} y={:.0}\n",
        camera.position().0,
        camera.position().1
    );
    engine.signal(&actor, Signal::Freeze).unwrap();
    actor.set_position(120.0, 90.0).unwrap();
    assert!(follow.update(&mut camera, (100, 80)).unwrap());
    trace.push_str(&format!(
        "frozen x={:.0} y={:.0}\n",
        camera.position().0,
        camera.position().1
    ));
    let viewport = Viewport {
        x: 10,
        y: 20,
        width: 40,
        height: 60,
    };
    camera = Camera::new(0.0, 0.0, Some(viewport)).unwrap();
    assert!(follow.update(&mut camera, (100, 80)).unwrap());
    trace.push_str(&format!(
        "viewport x={:.0} y={:.0}\n",
        camera.position().0,
        camera.position().1
    ));
    actor.kill().unwrap();
    assert!(!follow.update(&mut camera, (100, 80)).unwrap());
    assert!(!follow.is_following());
    trace.push_str(&format!(
        "ended x={:.0} y={:.0}\n",
        camera.position().0,
        camera.position().1
    ));
    assert_eq!(trace, include_str!("fixtures/camera_follow.txt"));
}

#[test]
fn follow_error_atomicity_independence_and_closed_context() {
    use malvado::{Engine, Signal, camera::Follow};
    let mut engine = Engine::new().unwrap();
    let actor = engine
        .spawn(Default::default(), |p| async move {
            loop {
                p.frame().await;
            }
        })
        .unwrap();
    let mut first = Follow::new(&actor).unwrap();
    let mut second = Follow::new(&actor).unwrap();
    let mut camera = Camera::new(3.0, 4.0, None).unwrap();
    let original = camera.position();
    assert_eq!(
        first.update(&mut camera, (0, 80)),
        Err(Error::InvalidArgument)
    );
    actor.set_position(f64::MAX, 0.0).unwrap();
    assert_eq!(
        first.update(&mut camera, (100, 80)),
        Err(Error::InvalidArgument)
    );
    assert_eq!(camera.position(), original);
    assert!(first.is_following());
    actor.set_position(120.0, 90.0).unwrap();
    engine.signal(&actor, Signal::Sleep).unwrap();
    engine.tick().unwrap();
    assert!(first.update(&mut camera, (100, 80)).unwrap());
    assert_eq!(camera.position(), (70.0, 50.0));
    first.clear();
    camera.set_position(7.0, 8.0).unwrap();
    assert!(!first.update(&mut camera, (100, 80)).unwrap());
    assert_eq!(camera.position(), (7.0, 8.0));
    assert!(second.is_following());
    assert!(second.update(&mut camera, (100, 80)).unwrap());
    assert_eq!(camera.position(), (70.0, 50.0));
    drop(engine);
    let original = camera.position();
    assert_eq!(second.update(&mut camera, (100, 80)), Err(Error::Closed));
    assert_eq!(camera.position(), original);
    assert!(second.is_following());
    assert!(matches!(Follow::new(&actor), Err(Error::Closed)));
    second.clear();
    assert!(!second.is_following());
}

#[test]
fn follow_does_not_retain_naturally_ended_process_state() {
    use malvado::{Engine, camera::Follow};
    use std::{cell::Cell, rc::Rc};
    struct CountDrop(Rc<Cell<u32>>);
    impl Drop for CountDrop {
        fn drop(&mut self) {
            self.0.set(self.0.get() + 1);
        }
    }
    let mut engine = Engine::new().unwrap();
    let drops = Rc::new(Cell::new(0));
    let owned = CountDrop(drops.clone());
    let actor = engine
        .spawn(Default::default(), |p| async move {
            let _owned = owned;
            p.frame().await;
            Ok(())
        })
        .unwrap();
    let mut follow = Follow::new(&actor).unwrap();
    let mut camera = Camera::new(3.0, 4.0, None).unwrap();
    engine.tick().unwrap();
    assert!(follow.update(&mut camera, (100, 80)).unwrap());
    engine.tick().unwrap();
    assert_eq!(drops.get(), 1);
    assert!(!follow.update(&mut camera, (100, 80)).unwrap());
    assert_eq!(camera.position(), (-50.0, -40.0));
    assert!(matches!(Follow::new(&actor), Err(Error::NotFound)));
}
