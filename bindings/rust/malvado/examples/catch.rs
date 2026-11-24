// Malvado 2; SPDX-License-Identifier: Apache-2.0
//! The Catch! game: catch a falling ball with an arrow-key player.
use malvado::{
    Error, Game,
    graphic::{Color, Geometry, Graphic, Pivot},
    window::{Key, Window},
};
use std::rc::Rc;

const WIDTH: f64 = 640.0;
const HEIGHT: f64 = 360.0;
const TICKS_PER_SECOND: f64 = 60.0;
const BLACK: Color = Color {
    r: 0,
    g: 0,
    b: 0,
    a: 255,
};
const CYAN: Color = Color {
    r: 0,
    g: 255,
    b: 255,
    a: 255,
};
const ORANGE: Color = Color {
    r: 255,
    g: 165,
    b: 0,
    a: 255,
};

fn main() -> Result<(), Error> {
    let mut game = Game::new()?;
    let window = Rc::new(Window::open(640, 360, "Catch!")?);

    // The player moves left and right and clamps its center to the playfield.
    let player = game.spawn(Default::default(), {
        let window = Rc::clone(&window);
        move |process| async move {
            let handle = process.current();
            while !window.key_down(Key::Escape) {
                let (mut x, y) = handle.position()?;
                let distance = 400.0 / TICKS_PER_SECOND;
                if window.key_down(Key::Left) {
                    x -= distance;
                }
                if window.key_down(Key::Right) {
                    x += distance;
                }
                handle.set_position(x.clamp(40.0, WIDTH - 40.0), y)?;
                process.frame().await;
            }
            Ok(())
        }
    })?;
    player.set_graphic(Graphic {
        geometry: Geometry::rectangle(80.0, 16.0, Pivot::Center)?,
        color: CYAN,
    })?;
    player.set_position(WIDTH / 2.0, HEIGHT - 40.0)?;

    // The ball falls, scores once per catch, and respawns above the screen.
    let ball = game.spawn(Default::default(), {
        let window = Rc::clone(&window);
        let player = player.clone();
        move |process| async move {
            let handle = process.current();
            let mut score = 0;
            while !window.key_down(Key::Escape) {
                let (mut x, mut y) = handle.position()?;
                y += 180.0 / TICKS_PER_SECOND;

                let caught = handle.collides_with(&player)?;
                if caught {
                    score += 1;
                }
                if caught || y > HEIGHT + 12.0 {
                    x = (x + 160.0) % WIDTH;
                    y = -12.0;
                }
                handle.set_position(x, y)?;
                handle.draw_text(&format!("Score: {score}"), (20, 20), 24, Color {
                    r: 255,
                    g: 255,
                    b: 255,
                    a: 255,
                })?;
                process.frame().await;
            }
            Ok(())
        }
    })?;
    ball.set_graphic(Graphic {
        geometry: Geometry::circle(12.0)?,
        color: ORANGE,
    })?;
    ball.set_position(80.0, 0.0)?;

    while game.process_count() != 0 && !window.should_close() {
        window.input_tick();
        game.tick()?;
        let frame = window.frame(BLACK)?;
        frame.process_graphics(&game)?;
    }
    Ok(())
}
