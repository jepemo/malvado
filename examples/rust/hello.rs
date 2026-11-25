// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::prelude::*;

async fn hello(p: ProcessContext) -> Result<(), Error> {
    loop {
        p.draw_text("Hello, Malvado!", (40, 40), 48, WHITE)?;
        p.frame().await;
    }
}

fn main() -> Result<(), Error> {
    let game = Game::windowed(800, 600, "Hello world")?;
    game.spawn(hello)?;
    game.run()
}
