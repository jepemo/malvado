// Malvado 2; SPDX-License-Identifier: Apache-2.0
//! Owned async processes, polled only by the native Malvado scheduler.
//!
//! A process owns its game state. Native handles expose copied snapshots, not
//! mutable references into a suspended future. Contexts and handles are local
//! to one thread. A cloned waker may cross threads; waking never calls C.
//!
//! ```no_run
//! use malvado::{Engine, Error, ProcessContext, SpawnOptions};
//! struct Counter(u32);
//! impl Counter {
//!     async fn run(mut self, process: ProcessContext) -> Result<(), Error> {
//!         for _ in 0..3 {
//!             self.0 += 1;
//!             process.frame().await;
//!         }
//!         Ok(())
//!     }
//! }
//! let mut engine = Engine::new()?;
//! engine.spawn(SpawnOptions::default(), |process| Counter(0).run(process))?;
//! while engine.process_count() != 0 { engine.tick()?; }
//! # Ok::<(), Error>(())
//! ```
mod engine;
mod task;

pub use engine::{Engine, Handle, ProcessContext, Snapshot, SpawnOptions};
pub use task::Frame;

/// Native failures preserve the C result code. A process error includes an
/// async return error or a contained panic; its native end reason is Error.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Error {
    Abi(&'static str),
    Closed,
    ForeignContext,
    Native(i32),
}
impl std::fmt::Display for Error {
    fn fmt(&self, out: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(out, "{self:?}")
    }
}
impl std::error::Error for Error {}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Signal {
    Sleep,
    Freeze,
    Wake,
    Kill,
}
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Status {
    Running,
    Sleeping,
    Frozen,
}
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum EndReason {
    Killed,
    Returned,
    Error,
}
