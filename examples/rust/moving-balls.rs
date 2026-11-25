// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::prelude::*;

const WIDTH: u32 = 640;
const HEIGHT: u32 = 360;
const RADIUS: f64 = 12.0;

async fn ball(p: ProcessContext, y: f64, mut speed: f64, color: Color) -> Result<(), Error> {
    p.set_graphic(circle(RADIUS).color(color))?;
    p.set_position(RADIUS, y)?;

    loop {
        let x = p.x()? + speed * p.delta();
        let right_edge = f64::from(WIDTH) - RADIUS;
        // Each process remembers its own direction across frame waits.
        if x < RADIUS || x > right_edge {
            speed = -speed;
        }
        p.set_x(x.clamp(RADIUS, right_edge))?;
        p.frame().await;
    }
}

fn main() -> Result<(), Error> {
    let game = Game::windowed(WIDTH, HEIGHT, "Moving balls")?;
    for (index, color) in [CYAN, ORANGE, GREEN].into_iter().enumerate() {
        let y = 90.0 * (index + 1) as f64;
        let speed = 120.0 + 65.0 * index as f64;
        game.spawn(move |p| ball(p, y, speed, color))?;
    }
    game.run()
}
