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
