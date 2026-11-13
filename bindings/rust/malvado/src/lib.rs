// Malvado 2; SPDX-License-Identifier: Apache-2.0
//! Experimental Rust gameplay SDK for Malvado 2.0.
//!
//! The SDK is functional and tested. Its public API may change in future 2.x
//! releases before Rust support is declared stable. Significant changes include
//! migration guidance. This label does not apply to the native C core.
//!
//! `Game` owns the current native context. Construction creates no window and
//! installs no host loop. Owned async processes use the same C scheduler as Lua
//! and C; window, input, audio and drawing services stay explicit.
//!
//! A process owns its game state. Native handles expose copied snapshots, not
//! mutable references into a suspended future. Contexts and handles are local
//! to one thread. A cloned waker may cross threads; waking never calls C.
//!
//! ```no_run
//! use malvado::{Game, Error, ProcessContext, SpawnOptions};
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
//! let mut game = Game::new()?;
//! game.spawn(SpawnOptions::default(), |process| Counter(0).run(process))?;
//! while game.process_count() != 0 { game.tick()?; }
//! # Ok::<(), Error>(())
//! ```
pub mod animation;
#[cfg(feature = "assets")]
pub mod asset;
#[cfg(feature = "assets")]
pub mod atlas;
#[cfg(feature = "graphics")]
pub mod audio;
pub mod camera;
pub mod collision;
pub mod commands;
#[cfg(feature = "graphics")]
pub mod drawing;
mod engine;
#[cfg(feature = "graphics")]
pub mod fade;
#[cfg(feature = "graphics")]
pub mod font;
#[cfg(feature = "assets")]
pub mod fpg;
pub mod graphic;
#[cfg(feature = "graphics")]
pub mod image;
mod resource;
mod task;
#[cfg(feature = "graphics")]
pub mod window;

pub use engine::{Game, Handle, LineageEntry, ProcessContext, Relation, Snapshot, SpawnOptions};
pub use resource::Resource;
pub use task::Frame;

/// Typed failures from the engine. Process includes an async return error or
/// a contained panic; its native end reason is Error.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Error {
    Abi(&'static str),
    /// Static diagnostic from the native asset parser.
    Asset(&'static str),
    /// Failure from an initialized or requested platform service.
    Platform(&'static str),
    Closed,
    ForeignContext,
    InvalidArgument,
    Overflow,
    NotFound,
    OutOfMemory,
    Busy,
    Process,
    QueueFull,
    QueueEmpty,
    BufferTooSmall,
    /// Native rectangle insertion rejected input, allocation, or capacity.
    WorldInsert,
    UnexpectedNative(i32),
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
/// Game access is local to its owner thread. Moving it to another thread is
/// rejected, also when a process is suspended.
/// ```compile_fail
/// let mut game = malvado::Game::new().unwrap();
/// std::thread::spawn(move || game.tick());
/// ```
/// Process futures own their state. A borrowed mutable local cannot escape
/// into a native process whose lifetime extends past the spawn call.
/// ```compile_fail
/// use malvado::{Game, SpawnOptions};
/// let game = Game::new().unwrap();
/// let mut state = 0;
/// game.spawn(SpawnOptions::default(), |_| async {
///     state += 1;
///     Ok(())
/// }).unwrap();
/// state += 1;
/// ```
/// Once state moves into a process, external code cannot keep another mutable
/// owner. This prevents aliases to data across an await.
/// ```compile_fail
/// use malvado::{Game, SpawnOptions};
/// let game = Game::new().unwrap();
/// let mut state = vec![1];
/// game.spawn(SpawnOptions::default(), move |p| async move {
///     let borrowed = &mut state;
///     p.frame().await;
///     borrowed.push(2);
///     Ok(())
/// }).unwrap();
/// state.push(3);
/// ```
/// A process token cannot recursively drive the native scheduler.
/// ```compile_fail
/// use malvado::{Game, SpawnOptions};
/// let game = Game::new().unwrap();
/// game.spawn(SpawnOptions::default(), |mut p| async move {
///     p.tick()?;
///     Ok(())
/// }).unwrap();
/// ```
mod ownership_contract {}

pub mod text;

#[cfg(any(feature = "assets", feature = "graphics"))]
mod native_path;
