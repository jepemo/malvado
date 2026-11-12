# Future feature roadmap

Status: proposals for evaluation after the first 2.0 release. No item on this
page is a release commitment, implemented capability, or assigned 2.x version.
The [version plan](../versions/README.md) owns current release gates.

The first 2.0 release completes and validates the engine already delivered by
alpha.6 and the C/Lua/Rust alignment in alpha.7. It does not require the complete
historical DIV/Fénix/BennuGD API, graphics modes, an editor, a new website, a full
teaching catalog, or minigames. Those candidates are organized here.

## Proposal index

| Proposal | Goal | Current state and dependencies |
| --- | --- | --- |
| [API candidate catalog](api-candidates.md) | Review historical calls and select useful modern capabilities | Existing pinned Bennu catalogs; DIV/Fénix coverage gaps are explicit. No blanket implementation selection. |
| [Visual properties, effects and shaders](visual-effects.md) | Small process effects, reusable shaders, then camera effects | Proposal; requires completed shared process drawing and resource ownership. |
| [Spatial attachments](spatial-attachments.md) | Explicit relative transforms and named visual points | Proposal; separate from delivered creator/owner genealogy. |
| [Bris live editing, scenes and reload](live-editing.md) | Inspect/edit processes, reload assets, author/reconcile flat scenes and evaluate code reload | Dependency milestones E1–E7; native state remains authoritative. No version assigned. |
| [Examples and minigames](examples.md) | Build a useful aligned C/Lua/Rust teaching catalog | Proposal; depends on delivered APIs and installed CLI workflows. |
| [Modes 7, 8 and 9](../specs/graphics-modes.md) | Perspective floors, sectors and a bounded 3D renderer | Existing design proposal; no fixed release numbers or complete 3D-engine promise. |
| [Graphical workshop](../specs/gui.md) | Projects, assets, process inspection and export | Existing proposal and experimental G0 tooling; production editor is not delivered. |
| [Asset extensions](../specs/assets.md#future-asset-review) | Additional Aseprite metadata, formats, FPG writing and runtime archives | Candidates; each format/direction requires a fidelity and ownership review. |
| [Public website](../website/README.md) | Complete reference navigation and presentation | Preparatory Astro/Starlight work exists; a site migration is not a 2.0 release gate. |
| LuaJIT evaluation | Measure a possible alternative Lua runtime | Research only; compare scheduler, coroutine cleanup, language/C API, memory, assets, exports and real game workloads before any runtime decision. |
| iOS feasibility | Evaluate another deployment target | Research only; toolchain, distribution, signing and device evidence are required before scheduling support. |

Use the existing specifications linked above as the single source for their
feature details. Do not create duplicate per-version copies of proposals.

## Promotion into a release

> API proposals for future alpha releases are provisional. Before implementing an alpha, reevaluate its API and technical design using the current engine architecture, previous implementation experience, and real game development needs. Prefer a simpler and more consistent design when justified.

This rule also applies to future 2.x feature milestones. The owner decides when
to schedule one and when to introduce a compatibility freeze. Breaking changes
within 2.x remain allowed until that explicit decision. Explain changes, update
contracts, tests and migration notes, and reject incompatible native SDKs before
unsafe calls. This policy does not authorize unrelated API changes during a task.

A proposal has a goal, bounded scope, API sketch, dependencies, observable
acceptance criteria, deferred work and open questions. Sketches describe intent;
strings such as `self.graph = "player.png"` are not claims about today's resource
assignment API. C uses idiomatic result/handle/ownership operations; Lua and Rust
use their own idioms over the same core behavior and scheduler.

Before promotion, select a real game use case, finish the required semantic
research, and define C/Lua/Rust and platform acceptance. One coherent capability
with tests, documentation and a small aligned example is a useful delivery unit.
No proposal requires another process scheduler, a generic ECS, a full editor or
an unrestricted rendering pipeline.

## Dependency order, without version numbers

1. Finish the current [2.0 release path](../versions/README.md), including platform automation.
2. Select a small teaching catalog and any genuinely needed API family from the candidate review.
3. Evaluate basic visual properties and simple relative 2D transforms independently.
4. Add a small process-effect set and named/animated 2D attachment points after their foundations pass.
5. Evaluate custom shader parameters and camera render targets as separate capabilities.
6. When Mode 9 foundations exist, adapt compatible effects and simple model attachment targets.
7. Evaluate skeletal sockets and shader reload only after their runtime dependencies exist.
8. Progress Bris independently through live properties, asset reload and flat scenes, then reconciliation; evaluate selective Lua restart and Rust rebuild/restart separately.

Modes 7/8/9, teaching work and these systems can progress independently where
there is no dependency. This order is a dependency guide, not a delivery queue.
Minigames follow the selected API capabilities and test their composition; they
do not force every exploratory API candidate to be implemented first.
