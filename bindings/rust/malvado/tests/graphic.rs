// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{
    collision::{Shape, Transform},
    graphic::{Geometry, Pivot},
};
#[test]
fn native_geometry_and_pivot_overlap() {
    let centered = Geometry::rectangle(20.0, 10.0, Pivot::Center).unwrap();
    let corner = Geometry::rectangle(20.0, 10.0, Pivot::TopLeft).unwrap();
    let point = Shape::circle(1.0).unwrap();
    let at = Transform {
        x: 18.0,
        y: 8.0,
        ..Default::default()
    };
    assert!(
        !centered
            .collider()
            .overlaps(Default::default(), point, at)
            .unwrap()
    );
    assert!(
        corner
            .collider()
            .overlaps(Default::default(), point, at)
            .unwrap()
    );
    let copy = corner;
    let _circle = Geometry::circle(5.0).unwrap();
    assert!(
        copy.collider()
            .overlaps(Default::default(), point, at)
            .unwrap()
    );
    for value in [0.0, -1.0, f64::NAN, f64::INFINITY, f64::MAX, 1e-40] {
        assert!(Geometry::circle(value).is_err());
        assert!(Geometry::rectangle(value, 1.0, Pivot::Center).is_err());
        assert!(Geometry::rectangle(1.0, value, Pivot::Center).is_err());
    }
    assert!(Geometry::circle(f32::MIN_POSITIVE.into()).is_ok());
    assert!(Geometry::circle((f32::MAX / 16.0).into()).is_ok());
}

#[test]
fn sprite_pivot_rotation_scale_and_boundaries() {
    let geometry = Shape::sprite((4, 4), (-2, 7)).unwrap();
    let point = Shape::circle(0.5).unwrap();
    let transform = Transform {
        x: 20.0,
        y: 20.0,
        angle: 90.0,
        scale: 2.0,
    };
    assert!(
        geometry
            .overlaps(
                transform,
                point,
                Transform {
                    x: 30.0,
                    y: 28.0,
                    ..Default::default()
                }
            )
            .unwrap()
    );
    assert!(
        !geometry
            .overlaps(
                transform,
                point,
                Transform {
                    x: 20.0,
                    y: 20.0,
                    ..Default::default()
                }
            )
            .unwrap()
    );
    assert!(Shape::sprite((i32::MAX, i32::MAX), (i32::MIN, i32::MAX)).is_ok());
    for size in [(0, 1), (1, 0), (-1, 1), (1, -1)] {
        assert!(Shape::sprite(size, (0, 0)).is_err());
    }
    let copy = geometry;
    assert!(
        copy.overlaps(
            transform,
            point,
            Transform {
                x: 30.0,
                y: 28.0,
                ..Default::default()
            }
        )
        .unwrap()
    );
}
