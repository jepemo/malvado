# Aligned examples for C, Lua, and Rust

Status: the example policy is adopted. The shared directory layout below is a
migration target, not the current tree. Alpha.7 A7.8 owns the first aligned
collection and its migration. Alpha.10 owns final public catalog selection and
presentation. This policy does not claim current three-language parity.

## What belongs in the collection

Use both feature examples and small minigames.

Feature examples teach one main concept with the smallest useful program. They
are the first place to learn process timing, signals, resource ownership,
collision, or a graphics operation. A reader should see the relevant engine
calls without navigating a framework or unrelated game logic.

A few minigames show how those concepts work together. Keep their rules,
controls, state, and assets small. They need a complete playable loop and
readable code, not the structure or content of a larger production game.
Do not turn every feature example into a game or use minigames as the only API
explanation. Performance benchmarks and engine verification fixtures have
separate purposes; they do not count as teaching examples or language ports.

## Alignment and readability rules

Each selected teaching example has C, Lua, and Rust implementations. They share
the same objective, observable behavior, controls, and assets where applicable.
Code remains idiomatic: C uses explicit state and cleanup, Lua uses its supported
process API, and Rust uses owned state and the safe API. Do not force the same
syntax, class structure, line count, or mechanical translation between them.
Use one recommended style per language in the teaching sequence; explain other
supported API forms in the reference rather than multiplying example variants.

Keep source small, clear, simple, and minimal. Use descriptive names and direct
control flow. Comments must be in English and explain the learning points,
intent, scheduler timing, ownership, or cleanup that a reader could miss.
Comments do not need to narrate each assignment or repeat the API reference.
Handle relevant errors without adding a general-purpose example framework.

One shared README explains the objective, prerequisites, three entry points,
commands, controls, expected output or behavior, and asset licenses. Show the
simplest useful example first and link to a minigame for composition. Document
language-specific setup without duplicating the common lesson three times.
Source is authoritative for site snippets; do not maintain separate manual
copies of the same runnable program.

A missing language API or implementation is an explicit open gap. Do not fill
it with a different algorithm, hidden engine implementation, or a more complex
language-specific showcase and call that equivalent. Do not advertise a topic
as aligned until all three programs and their checks are complete.

## Target directory layout

Group by topic so the three implementations and their shared contract are
visible together. The two categories describe their teaching purpose:

```text
examples/
  README.md
  features/
    <topic>/
      README.md
      assets/          # only when needed; shared, licensed sources
      c/               # main.c and required build configuration
      lua/             # main.lua
      rust/            # Cargo.toml and src/main.rs
  games/
    <game>/
      README.md
      assets/          # only when needed
      c/
      lua/
      rust/
```

Create only populated directories during implementation. CMake remains the C
build owner; Rust examples are Cargo projects using the delivered SDK workflow.
Bindings stay in `bindings/`. Public Rust programs currently in the binding's
Cargo examples must move into the selected root collection. Library doctests
and verification fixtures can stay with their owning component. Do not keep
separate runnable source copies just to preserve old Cargo example commands.

A language-first layout would keep languages easy to browse, but spread a
topic's contract, assets, and parity review across separate catalogs. Grouping
by topic makes alignment and shared documentation easier to maintain. The root
index can still provide direct C, Lua, and Rust command links.

## Current inventory and migration constraints

The reviewed tree contains 12 `examples/c/*/main.c` programs, 22 top-level Lua
example folders, three Lua counterparts inside C folders, and two Rust programs
in `bindings/rust/malvado/examples/`. These counts describe the current migration
baseline, not a selected final catalog.

| Current group | Examples of current paths | Migration consideration |
| --- | --- | --- |
| Native feature examples | `examples/c/processes/`, `control/`, `inspection/`, `resources/` | Consolidate related lessons and add matched Lua/Rust implementations. |
| Lua feature examples | `examples/processes/`, `process-tree/`, `collision/`, `world-collision/`, `camera-follow/` | Map behavior to a shared topic; do not claim Rust graphical parity before A7.6. |
| Existing paired programs | `examples/c/processes/`, `parity/`, `timing-game/` | Preserve existing traces and checks; review readability before catalog selection. |
| Binding-local Rust examples | `bindings/rust/malvado/examples/counter.rs`, `world.rs` | Move selected teaching sources under root examples and update Cargo smoke commands and installed-SDK checks. |
| Game/demo candidates | `examples/space_asteroids/`, `demo/` | Review suitability and size; selection does not automatically approve ports of every existing demo. |
| Performance program | `examples/bench/` | Keep its verification until an explicit migration to tooling; do not require a teaching-game port merely for symmetry. |

The current root README, CMake, mise tasks, test drivers, API guides, and Cargo
smoke script reference these paths. The CLI exports a project directory and
currently expects `main.c` or `main.lua` there. Moving files into language
subfolders without updating project roots and shared asset handling can break
run/export or omit assets. Resolve those paths in the delivered CLI/SDK workflow;
do not invent another manifest or silently rely on an undocumented working
directory. Verify exported assets and licenses on an installed, relocated SDK.

## Milestone responsibilities

- **Alpha.7 A7.8:** map existing programs to the required hello, circle/square,
  process-family/bullets, sequential boss, and world-movement lessons already in
  that gate. Select the compact game required there. Give the selected topics
  and game matched C/Lua/Rust versions, simple commented source, shared README,
  assets, and automated evidence. Migrate their root paths with builds, tests,
  docs, CLI commands and packaging. Keep A7.8 open until the full required set
  and all three variants are verified. This policy does not remove its existing
  Rust example or polished game deliverables.
- **Alpha.8:** provide aligned examples for each newly delivered API family.
  Extend existing topics when that preserves one clear lesson; do not duplicate
  the catalog for each release or language.
- **Alpha.10:** finalize the public catalog and learning order against the
  complete API. Record which existing demos are selected, combined, retained as
  verification, or retired, with replacement links and license/asset handling.
  Publish a coverage table and link each topic's three language versions from
  the site. This is editorial selection and consolidation, not a new API gate.

No additional alpha is needed just to decide the catalog. Alpha.10 already owns
public examples and documentation. Choosing the initial A7.8 set cannot wait
for alpha.10, because its runnable examples and game are existing alpha.7
acceptance requirements. Existing programs outside that selected set keep their
current verified paths until their disposition is recorded; they do not count
as aligned entries in the new catalog.

## Acceptance for an aligned example

Compile C and Rust and run all three versions to a bounded completion or smoke
checkpoint. Check the same trace or observable result. For interactive programs,
check matching controls and the relevant automated graphics/input behavior.
Verify cancellation, cleanup, shared assets, and installed/exported execution
where applicable. A build alone does not establish equivalence or teachability.

Record the three entry points, commands, expected behavior and test identifiers
in the shared README or catalog matrix. Keep C-only and Lua workflows usable
without Cargo. Keep headless examples independent of graphics services. Runtime
and package checks stay in the feature's alpha; manual full user flows and
physical-device checks remain in beta.

Earlier example proposals included masks, GUI workflows and unselected formats.
They do not establish available engine capabilities or new 2.0 requirements.
Use the accepted [version scope](../versions/README.md),
[API inventory](../api/family-review.md), and
[asset contracts](../specs/assets.md) when selecting programs.
