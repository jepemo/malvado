# Multi-language architecture

Status: native boundary and installed sys SDK implemented in alpha.7 after alpha.6 acceptance. See the [SDK evidence](../versions/2.0.0-alpha.7-rust-sdk.md). The owned-future adapter and safe process API are implemented; see the [scheduler proof](../versions/2.0.0-alpha.7-scheduler.md). Native lifecycle, generic resources and platform/graphics integration are complete; see the [family acceptance audit](../versions/2.0.0-alpha.7-integration.md). The [final acceptance](../versions/2.0.0-alpha.7-acceptance.md) records the passing host CI, verified candidate packages and completed milestone. The owner has chosen a C-first engine, one scheduler, and first-class Lua, C, and Rust; struct-based async processes are the preferred experience, not finalized syntax.

## Boundary and existing baseline

C11 `src/core`, public `src/include/malvado`, platform adapters, `bindings/lua`, and the existing scheduler remain authoritative. [Process inventory](../api/process-inventory.md) defines identity, owned handles, context-local IDs, lineage, messages and cleanup. [Resource lifetime](../api/resource-lifetime.md) defines ownership. Alpha.6 is accepted and delivers shape overlap, static world geometry, process graphics and Lua queries. The [alpha.7 baseline](../versions/2.0.0-alpha.7-baseline.md) records which behavior still lives in the Lua binding. Rust wraps only capabilities actually accepted by alpha.6. Check public C exposure before promising Lua parity; add missing C contracts rather than duplicating algorithms in Rust.

Lua and Rust adapters call the public C interface; neither owns another game scheduler. C-only builds need no Lua, Rust, Cargo, or raylib. Lua-only builds need no Rust. Each process and resource belongs to one engine context; handles crossing contexts fail validation. Mixed C/Lua/Rust processes in a single context are a possible integration test, not a promise of public dynamic interoperability. Cross-language types, payload encoding, conversions, ownership and shutdown need separate contracts. No arbitrary downcasting or direct Lua access to Rust state is implied.

## Crates and linking

Recommend `bindings/rust/` as a Cargo workspace: `malvado-sys` for explicit unsafe FFI and native discovery, `malvado` for safe APIs, and `malvado-macros` only after the scheduler prototype proves a need. A safe wrapper cannot promise safety unless all native contracts and thread restrictions are enforced. Resources use RAII to release owned native references; engine-owned and process-owned resources remain subject to native transfer and termination rules. Images, atlas regions, sprites, process graphs, shapes, transforms, text/fonts, immediate drawing, keyboard/mouse, sound/music and collision world require an inventory mapping and tests. No GPU readback or duplicate collision implementation.

| Strategy | Benefits | Costs and constraints |
| --- | --- | --- |
| Installed SDK | Same C API as external C consumers; no second native engine | SDK discovery, architecture, ABI and debug/release compatibility must be explicit on each host |
| Build native sources in build.rs | Convenient checkout bootstrap | Requires CMake/compiler and source distribution; duplicates builds, target flags and dependencies; risks multiple engines |
| Hybrid, recommended | CI installs a CMake-built SDK to a temporary prefix; Cargo links it; distributed SDK uses the same discovery | Requires a documented prefix and matching libraries; no silent source download or implicit fallback |

Use CMake for native products and Cargo for Rust; mise tasks orchestrate optionally. Compare static dependency propagation and dynamic loader paths per supported macOS/Linux host. Windows and Android toolchains, NDK, ABI, and packaging are gates in alpha.8, not inferred from desktop. Cross-compilation and crates.io are not promised. Test installed external consumers without checkout paths, and reject mismatch before unsafe calls. Check architecture, build profile and dependencies; avoid two copies of the engine in one process. The player gets an executable, assets and notices, not compilers or SDKs.

For bindings, handwritten declarations avoid libclang and generated noise but need systematic ABI/signature checks. bindgen reduces declaration drift but adds libclang and cross-target reproducibility concerns; checked-in generated bindings still require refresh checks. Recommend a small manually maintained surface with compile-time C assertions and CI comparison, revisiting bindgen if coverage makes drift unmanageable.

## Repository and compatibility

Keep `src/` and CMake install/include paths in alpha.7. The complete topic-based teaching catalog remains future work under the [example migration plan](../examples/README.md); existing programs keep their verified paths until that migration. The owner approved one bounded layout change: move `src/binding/` to `bindings/lua/` beside the Rust workspace, tracked by A7.11. The relocation and A7.11 acceptance checks are complete. Grouping language adapters makes their responsibility clear; each keeps its own build tool and internal structure. Do not move the whole tree or split the CLI for symmetry. Group each selected feature/game with its C/Lua/Rust variants, shared README and licensed assets under `examples/`; do not keep public Rust programs inside the binding. A wider engine reorganization still needs a separate migration, exporter, install and history plan. External games remain independent Cargo projects.

The planned layout is:

```text
src/core/                 shared native behavior and scheduler
src/include/malvado/      public C API
src/platform/             native platform adapters
bindings/lua/             Lua adapter, built by CMake
bindings/rust/            Rust workspace, managed by Cargo
```

The C API stays with the core; it is not a separate language adapter. Keep the Lua move separate from extracting shared behavior for Rust parity. See the [migration steps](../development/rust-migration.md#lua-binding-relocation-a711) for acceptance and verification.

Recommend matching crate and engine alpha versions initially, with an explicit native API/ABI version query and compatibility check. The C ABI is not frozen: exact supported native version and build configuration must be checked; reject unknown combinations. CLI and SDK releases identify compatible versions, and exported games ship their required native libraries and notices. Before crates.io publication verify licensing, source availability, dependency discovery, ABI matrix, documentation, clean consumer builds and release ownership. No publication is authorized here.

Future features update the C contract, Lua and Rust adapters, language tests, documentation and relevant examples together. Differences must be explicit. Existing 2D `z` remains draw depth; `x`, `y`, `angle`, `size` retain their native semantics. Future modes decide spatial semantics separately; no mandatory `position` field.

See [scheduler design](../specs/rust-processes.md), [migration plan](../development/rust-migration.md), and [alpha.7 tasks](../versions/2.0.0-alpha.7.md).
