# Teaching examples and minigames

Status: future proposal. See the [runnable examples](../../examples/README.md)
for current commands and the [example policy](../../AGENTS.md#example-policy)
for contribution rules.

## Goal and scope

Create a small catalog that teaches the delivered engine clearly in C, Lua and
Rust. Select lessons from learning objectives, not from the current folder list.
Replace, combine or retire existing examples when that produces better lessons.
Keep existing verified paths until replacements update builds, tests, links,
asset resolution, CLI exports and licenses together.

Candidate lessons: hello/startup; rectangles/circles; process timing; parent and
independent bullets; sequential boss behavior; signals and cleanup; resources;
input; sprites and animation; collision and world movement; camera/region/scroll;
text and audio. Select a minimal useful sequence before implementation.

## Proposed layout and migration

Group lessons under `examples/features/<topic>/` and minigames under
`examples/games/<game>/`. Each topic contains `c/`, `lua/` and `rust/`
entry points, one README and an `assets/` directory when the programs
share assets. Keep C builds in CMake and Rust builds in Cargo. Use the
runnable sources for documentation snippets.

Review the existing examples and the public examples inside the Rust binding.
Move selected programs with their build, test, documentation and export
commands. Keep library doctests and verification fixtures with their
components. Decide where benchmark programs belong before moving them.

Check project entry points, working directories and relative asset paths in
both source and exported games. Verify shared assets and licenses with a
relocated SDK. Retire replaced sources after their references and checks
point to the new paths.

## API sketch

Use only delivered APIs. A lesson such as an animated actor should show resource
loading, a process and a frame wait directly in each language. No shared example
framework or extra public API is required. See current [Lua](../api/lua-processes.md)
and [Rust](../api/rust-processes.md) contracts and the [C contract](../api/c-contract.md).

## Dependencies and acceptance

Require the relevant APIs and installed create/run/export workflow. Each chosen
topic has three idiomatic implementations, one shared README, observable aligned
behavior, common controls and licensed shared assets. Compile C/Rust, run all
three with bounded completion, compare traces or pixels/input, and verify cleanup
and exported execution where applicable. Record missing variants as incomplete.

A later small minigame set demonstrates a complete readable play loop. It follows
the selected API work; an endless catalog of historical candidates is not its
prerequisite. The former compact Rust game requirement becomes an aligned
C/Lua/Rust minigame proposal here, not an alpha.7 gate.

## Deferred work and open questions

The [startup, process and image lessons](../../examples/features/README.md)
are already available in all three languages. Decide which other programs
teach well, which belong in verification or benchmarks, how shared assets
resolve in exported projects, and which small game best demonstrates how
the APIs work together.
