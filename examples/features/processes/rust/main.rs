// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{Error, Game, ProcessContext};

async fn work(process: ProcessContext, name: &str) -> Result<(), Error> {
    println!("{name}: 1");
    process.frame().await; // Resume on the next activation.
    println!("{name}: 2");
    Ok(())
}

fn main() -> Result<(), Error> {
    let mut game = Game::new()?;
    game.spawn(|process| work(process, "first"))?;
    game.spawn(|process| work(process, "second"))?;
    while game.process_count() != 0 {
        game.tick()?;
    }
    Ok(())
}
