# Future feature roadmap

Each proposal describes a possible feature or research topic. Its document
holds the design, dependencies and verification ideas.

## Proposal index

| Proposal | Goal | Current state and dependencies |
| --- | --- | --- |
| [WebAssembly / HTML5 export](webassembly.md) | Export games to static web hosting through the existing CLI | C runtime proof, then C/Lua export and browser reliability. Rust needs a separate toolchain proof. |
| [Rust API Ergonomics and DIV-Style Processes](rust-ergonomics.md) | Managed gameplay and named async functions over the owned-future adapter | Baseline delivered in the current tree. Typed declarations remain research. |
| [API candidate catalog](api-candidates.md) | Review historical calls and select useful modern capabilities | Existing pinned Bennu catalogs; DIV/Fénix coverage gaps are explicit. No blanket implementation selection. |
| [Visual properties, effects and shaders](visual-effects.md) | Small process effects, reusable shaders, then camera effects | Proposal; requires completed shared process drawing and resource ownership. |
| [Optional 2D and 3D process physics](physics.md) | Add simulation as a process capability while retaining lightweight collision APIs | Unassigned P1–P5; backend proof, integrated 2D, selected helpers, then scene-dependent 3D and optional tooling. [Library comparison](physics-libraries.md). |
| [Spatial attachments](spatial-attachments.md) | Explicit relative transforms and named visual points | Proposal; separate from delivered creator/owner genealogy. |
| [malvado-gui live editing, scenes and reload](live-editing.md) | Inspect/edit processes, reload assets, author/reconcile flat scenes and evaluate code reload | Dependency milestones E1–E7; native state remains authoritative. No version assigned. |
| [Examples and minigames](examples.md) | Build a useful aligned C/Lua/Rust teaching catalog | Proposal; depends on delivered APIs and installed CLI workflows. |
| [Modes 7, 8 and 9](graphics-modes.md) | Perspective floors, sectors and a bounded 3D renderer | Existing design proposal; no fixed release numbers or complete 3D-engine promise. |
| [Graphical workshop](gui.md) | Projects, assets, process inspection and export | Existing proposal and experimental G0 tooling; production editor is not delivered. |
| [Project tools](project-tools.md) | Project metadata, templates and SDK selection | Reuse the existing CLI and define one project workflow before adding commands. |
| [Asset extensions](asset-formats.md) | Additional Aseprite metadata, formats, FPG writing and runtime archives | Candidates; each format/direction requires a fidelity and ownership review. |
| [Documentation](#documentation) | Teach Malvado through learning guides, tutorials, and runnable examples | The public site uses Astro and Starlight. See the [website guide](../website/README.md). |
| [External Lua module and LuaRocks](../development/distribution.md#optional-external-lua-module) | Let projects use Malvado from an existing Lua interpreter | Requires a loadable C binding, Lua ABI checks and a tested rockspec. |
| LuaJIT evaluation | Measure a possible alternative Lua runtime | Research only; compare scheduler, coroutine cleanup, language/C API, memory, assets, exports and real game workloads before any runtime decision. |
| iOS feasibility | Evaluate another deployment target | Research only; toolchain, distribution, signing and device evidence are required before scheduling support. |

## Documentation

Use the [writing guide and templates](../development/api-documentation.md)
for new and updated documentation.

- Establish canonical examples and shared assets in [#63](https://github.com/jepemo/malvado/issues/63).
- Rewrite onboarding and tool guides in [#64](https://github.com/jepemo/malvado/issues/64).
- Add Learn coverage for advanced [input](https://github.com/jepemo/malvado/issues/42),
  [audio mixing](https://github.com/jepemo/malvado/issues/40),
  [spatial queries](https://github.com/jepemo/malvado/issues/38) and
  [runtime graphics libraries](https://github.com/jepemo/malvado/issues/44)
  as those APIs become available.
- Build on the [Dodge! tutorial](../site/tutorials/dodge.md) with future tutorials.
  Explore Asteroids for rotation and wraparound, a platformer for jumping
  and terrain, a top-down adventure for rooms and interaction, and a
  shoot 'em up for waves and projectiles.
- Expand the [examples catalog](https://github.com/jepemo/malvado/issues/68) and add [interactive playgrounds](https://github.com/jepemo/malvado/issues/69), including the scheduler.
- Apply the function template across the Lua API reference in [#70](https://github.com/jepemo/malvado/issues/70).
- Enforce reference coverage, runnable examples, and website checks in CI in [#71](https://github.com/jepemo/malvado/issues/71).
- Add [patterns](https://github.com/jepemo/malvado/issues/72) for game state, spawning, communication, and levels as verified examples become available.

## Promotion into a release

Choose a proposal when it is ready for implementation. Review it against a
real game use case and the current engine, then decide what to deliver.
Define the C, Lua and Rust paths and the applicable tests. Create a version
page in the [version plan](../versions/README.md) once its scope is agreed.

Record and fix reported defects as they arise. Include completed fixes in
the relevant release.
