// Malvado 2; SPDX-License-Identifier: Apache-2.0
use malvado::{Engine, Error, ProcessContext, SpawnOptions};

struct Counter {
    value: u32,
}
impl Counter {
    async fn run(mut self, process: ProcessContext) -> Result<(), Error> {
        for _ in 0..3 {
            self.value += 1;
            println!("counter: {}", self.value);
            process.frame().await;
        }
        Ok(())
    }
}

fn main() -> Result<(), Error> {
    let mut engine = Engine::new()?;
    engine.spawn(SpawnOptions::default(), |process| {
        Counter { value: 0 }.run(process)
    })?;
    while engine.process_count() != 0 {
        engine.tick()?;
    }
    Ok(())
}
