// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{Error, Game};

fn main() -> Result<(), Error> {
    let _game = Game::new()?;
    println!("ready");
    Ok(())
}
