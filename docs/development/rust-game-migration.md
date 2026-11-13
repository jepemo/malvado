# Rust Engine to Game migration

The canonical Rust context owner is now `malvado::Game`. Replace `Engine` imports,
type annotations and constructor calls with `Game`. Local variables can use the
name `game`. Update CLI-generated projects that still use the old name.

```rust
use malvado::{Error, Game, SpawnOptions};

fn main() -> Result<(), Error> {
    let mut game = Game::new()?;
    game.spawn(SpawnOptions::default(), |process| async move {
        process.frame().await;
        Ok(())
    })?;
    while game.process_count() != 0 {
        game.tick()?;
    }
    Ok(())
}
```

`Engine` is removed, with no compatibility alias. The owner approved this small
pre-2.0 breaking change under the development-version policy. One public name
keeps examples, generated projects and reference documentation consistent.
Historical alpha acceptance records retain the name used at their tested
revision; completed milestones are not reopened.

The internal `engine` module and native runtime remain in place. No C or Lua
symbol changes. The native ABI revision, layouts and release/SDK mismatch checks
are unchanged. Rebuild Rust code against the updated installed crate sources;
this Rust source rename does not require a native ABI revision bump.

Construction, the explicit host loop, window service, clock defaults, spawning,
process identity, cancellation, resources and shutdown keep their existing
contracts. `Game::new()` does not open a window. There is no new builder,
executor, macro, process declaration style or `Game::run()` method.

The Rust gameplay API is [experimental](../api/rust-support.md). That policy
allows future approved changes with migration guidance; it does not weaken
safety, tests or the shared native scheduler contract.

## Verification

Local macOS verification passed:

- `mise run test-rust`: Debug/Release safe API, scheduler/lifecycle/shutdown,
  ABI diagnostics, installed external Cargo consumer, Clippy and formatting.
- `mise run test-c`: 75 native checks without Lua or raylib.
- `mise run test-headless`: 203 C/Lua headless checks.
- `mise run test-rust-cli`: generated `Game` template, check/build/run, exports,
  assets and a player without SDK or development tools.
- `mise run test-rust-graphics`: Debug/Release graphics/assets tests, independent
  graphics-only builds, installed consumers, Clippy and optional-component/ABI errors.
- All 37 current Rust API guide snippets compiled and ran in both profiles.
- `mise run test-docs`, targeted roadmap/version links and strict MkDocs build.

Run graphical verification serially on this host. Concurrent graphical suites
encountered GLFW monitor-discovery crashes; serial reruns passed. No renderer
or scheduler change was made to address that environment-dependent failure.
Windows and Android acceptance is not newly claimed by this rename; retain the
recorded platform matrix and its current limitations.
