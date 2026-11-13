// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::collision::{Rect, Shape, World};
use malvado::{Error, Game};

fn main() -> Result<(), Error> {
    let mut world = World::new()?;
    world.add_rect(Rect {
        x: 10.0,
        y: 0.0,
        width: 10.0,
        height: 10.0,
    })?;
    let shape = Shape::circle(1.0)?;
    let game = Game::new()?;
    let actor = game.spawn(Default::default(), |p| async move {
        p.frame().await;
        Ok(())
    })?;
    actor.set_position(0.0, 5.0)?;
    actor.bind_collider(Some(shape))?;
    // The native operation updates the process without activating its future.
    let contact = world.move_selected(&actor, 100.0, 0.0)?;
    let (x, y) = actor.position()?;
    println!("x={x} y={y} hit={:?}", contact.hit);
    assert_eq!(x, 9.0);
    assert_eq!(contact.hit, Some(1));
    Ok(())
}
