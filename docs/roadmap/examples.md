# Teaching examples and minigames

Status: future proposal, moved from alpha.7 A7.8 by the owner scope decision.
No version is assigned. See the [example policy](../examples/README.md) for layout
and alignment rules. The move does not mark the old deliverables complete.

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

A full catalog and polished games are outside the first 2.0 release. Beta still
needs minimal tested getting-started examples in all three languages. Decide
which current programs teach well, which remain verification/benchmarks, how
shared assets resolve in exported projects, and which small game best validates
composition. Coordinate publication with future site work.
