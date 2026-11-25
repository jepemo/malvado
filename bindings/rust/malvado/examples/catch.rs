// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::prelude::*;

const WIDTH: u32 = 640;
const HEIGHT: u32 = 360;

async fn player(p: ProcessContext) -> Result<(), Error> {
    p.set_graphic(rect(80.0, 16.0).color(CYAN))?;
    p.set_position(f64::from(WIDTH) / 2.0, f64::from(HEIGHT) - 40.0)?;

    loop {
        let mut x = p.x()?;
        let distance = 400.0 * p.delta();
        if p.key_down(Key::Left) {
            x -= distance;
        }
        if p.key_down(Key::Right) {
            x += distance;
        }
        p.set_x(x.clamp(40.0, f64::from(WIDTH) - 40.0))?;
        p.frame().await;
    }
}

async fn ball(p: ProcessContext, player: Handle) -> Result<(), Error> {
    p.set_graphic(circle(12.0).color(ORANGE))?;
    p.set_position(80.0, 0.0)?;
    let mut score = 0;

    loop {
        p.set_y(p.y()? + 180.0 * p.delta())?;
        let caught = p.collides_with(&player)?;
        if caught {
            score += 1;
        }
        if caught || p.y()? > f64::from(HEIGHT) + 12.0 {
            p.set_position((p.x()? + 160.0) % f64::from(WIDTH), -12.0)?;
        }
        p.draw_text(&format!("Score: {score}"), (20, 20), 24, WHITE)?;
        p.frame().await;
    }
}

fn main() -> Result<(), Error> {
    let game = Game::windowed(WIDTH, HEIGHT, "Catch!")?;
    // The scheduler activates the player first at equal priority.
    let player = game.spawn(player)?;
    game.spawn(move |p| ball(p, player))?;
    game.run()
}
