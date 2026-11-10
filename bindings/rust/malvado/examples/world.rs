// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::Error;
use malvado::collision::{Rect, Shape, Transform, World};

fn main() -> Result<(), Error> {
    let mut world = World::new()?;
    world.add_rect(Rect {
        x: 10.0,
        y: 0.0,
        width: 10.0,
        height: 10.0,
    })?;
    let shape = Shape::circle(1.0)?;
    let mut position = Transform {
        y: 5.0,
        ..Default::default()
    };
    let contact = world.move_by(shape, &mut position, 100.0, 0.0)?;
    println!("x={} y={} hit={:?}", position.x, position.y, contact.hit);
    assert_eq!(position.x, 9.0);
    assert_eq!(contact.hit, Some(1));
    Ok(())
}
