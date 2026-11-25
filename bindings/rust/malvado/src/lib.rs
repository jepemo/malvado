// Malvado 2; SPDX-License-Identifier: Apache-2.0
//! Experimental Rust gameplay SDK for Malvado 2.0.
//!
//! The SDK is functional and tested. Its public API may change in future 2.x
//! releases before Rust support is declared stable. Significant changes include
//! migration guidance. This label does not apply to the native C core.
//!
//! Start windowed gameplay with the `graphics` feature and the prelude.
//! Owned async processes run on the native C scheduler.
//!
#![cfg_attr(
    feature = "graphics",
    doc = r#"
```no_run
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
```
"#
)]
//! Use `Game::new()` for a headless game or an explicit host loop. Handles and
//! contexts are local to one thread. A cloned waker may cross threads; waking
//! never calls C.
//!
//! ```
//! use malvado::prelude::*;
//! async fn worker(p: ProcessContext) -> Result<(), Error> {
//!     p.frame().await;
//!     Ok(())
//! }
//! let mut game = Game::new()?;
//! game.spawn(worker)?;
//! while game.process_count() != 0 { game.tick()?; }
//! # Ok::<(), Error>(())
//! ```
pub mod android;
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
#[cfg(feature = "graphics")]
mod runner;
mod task;
#[cfg(feature = "graphics")]
pub mod window;

pub use engine::{Game, Handle, LineageEntry, ProcessContext, Relation, Snapshot, SpawnOptions};
pub use resource::Resource;
pub use task::Frame;

/// Common types, shapes, and colors for gameplay code.
pub mod prelude {
    #[cfg(feature = "graphics")]
    pub use crate::window::{Key, MouseButton, PadAxis, PadButton};
    pub use crate::{
        Error, Game, Handle, ProcessContext, Signal, SpawnOptions,
        graphic::{BLACK, BLUE, CYAN, Color, GREEN, ORANGE, RED, WHITE, circle, rect},
    };
}

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
/// use malvado::Game;
/// let game = Game::new().unwrap();
/// let mut state = 0;
/// game.spawn(|_| async {
///     state += 1;
///     Ok(())
/// }).unwrap();
/// state += 1;
/// ```
/// Once state moves into a process, external code cannot keep another mutable
/// owner. This prevents aliases to data across an await.
/// ```compile_fail
/// use malvado::Game;
/// let game = Game::new().unwrap();
/// let mut state = vec![1];
/// game.spawn(move |p| async move {
///     let borrowed = &mut state;
///     p.frame().await;
///     borrowed.push(2);
///     Ok(())
/// }).unwrap();
/// state.push(3);
/// ```
/// A process token cannot recursively drive the native scheduler.
/// ```compile_fail
/// use malvado::Game;
/// let game = Game::new().unwrap();
/// game.spawn(|mut p| async move {
///     p.tick()?;
///     Ok(())
/// }).unwrap();
/// ```
mod ownership_contract {}

pub mod text;

mod native_path;
