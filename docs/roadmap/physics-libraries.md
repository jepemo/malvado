# Physics library candidates

Status: research snapshot, 2026-11-16. This supports the
[process physics proposal](physics.md), not a dependency selection. Upstream
features below describe the reviewed development branches, not a promised
Malvado feature set. Pin a release/commit and recheck all claims in P1/P4.
No candidate was compiled, benchmarked or integrated for this planning task.

## 2D candidates

| Candidate and primary source | Features, maturity and license | Integration and maintenance assessment |
| --- | --- | --- |
| [Physac, raysan5 tree](https://github.com/raysan5/physac), [header](https://github.com/raysan5/physac/blob/master/src/physac.h) | Small C single-header solver with circles/polygons, forces, materials and configurable fixed pools; zlib/libpng license. The header identifies v1.1 and a 2021 revision. Its old README describes threading, while the header says threading was removed. | Small build footprint, but global state/pools complicate independent contexts and scalability. The header's `PHYSAC_AVOID_TIMMING_SYSTEM` allows caller-controlled stepping; verify actual timing and raylib type dependencies. Attractive for tiny demos, less convincing for sustained feature growth. No Box2D-level joint/CCD parity should be inferred. |
| [Box2D](https://github.com/erincatto/box2d), [manual](https://box2d.org/documentation/) | Established 2D lineage; current main is C17, MIT. Advertises circles, capsules, convex polygons/chains, CCD, sensors, filtering, contacts, joints and character movers, with data-oriented storage, SIMD and parallel work. | Opaque IDs and explicit world stepping fit a private C adapter. CMake separates core from C++ sample tooling. Windows/Linux/macOS builds are documented. Strong candidate for stacks and larger scenes, but measure overhead for small games and pin a stable API rather than following main. |
| [Chipmunk2D](https://github.com/slembcke/Chipmunk2D), [current upstream](https://codeberg.org/slembcke/Chipmunk2D) | Established C99, MIT alternative; circles, convex polygons, segments, sensors/filtering, queries and joints. Provides tree/hash broad phases and sleeping. The project announces a move to Codeberg; an old GitHub activity view is insufficient maintenance evidence. | Small C dependency, explicit spaces and callbacks suit a C adapter. CMake and desktop build instructions exist, but revalidate current MSVC and release packaging. Worth comparing API effort and shape/contact behavior with Box2D; no unverified CCD or cross-platform determinism promise. |

Physac is not selected simply because it is close to raylib. Audit the exact
fork and header used; resolve README/source differences before testing.
For Box2D and Chipmunk, verify single-threaded Emscripten builds and numerical
behavior rather than assuming portable C proves browser support. Physac also
needs global-state isolation and memory-limit evidence. All three would use
Malvado's C boundary for Lua and Rust, not separate language physics libraries.

## 3D candidates

| Candidate and primary source | Features, maturity and license | Integration and maintenance assessment |
| --- | --- | --- |
| [Box3D](https://github.com/erincatto/box3d) | Erin Catto's C17, MIT engine. Main advertises rigid bodies, hulls/capsules/spheres, meshes/height fields, CCD, sensors, contact events, character movement and joints. The [initial v0.1.0 release](https://github.com/erincatto/box3d/releases/tag/v0.1.0) is explicitly labeled alpha by its author; stability needs evidence. | CMake core needs only the C runtime and Unix math library; sample tooling is separate C++20. Documents Windows/Linux/macOS and an Emscripten build, with SIMD configuration. Low language friction, but release history, API churn, regression coverage and production use need review before adoption. |
| [Jolt Physics](https://github.com/jrouwe/JoltPhysics), [simulation documentation](https://jrouwe.github.io/JoltPhysicsDocs/5.5.0/) | C++17, MIT; rigid-body shapes, CCD, sensors, constraints, terrain and character support. Upstream cites shipped games and supports desktop platforms plus a separate WebAssembly project. Designed for concurrent work; deterministic settings have explicit limits. | Requires a private C shim, C++ linking and allocator/job-system decisions. Strong production/scaling candidate for a substantial 3D world; more integration machinery than a C library. Keep callbacks off language runtimes and benchmark single-threaded builds first. |
| [Bullet Physics](https://github.com/bulletphysics/bullet3) | Established C++ SDK with rigid-body collision/dynamics and broader simulation features. Permissive zlib license except files with separate notices. Documents Windows/Linux/macOS testing. | Select a small CPU rigid-body subset through a private C shim; do not bring in PyBullet, robotics or GPU components. Broad capabilities can increase configuration and maintenance cost. Browser compilation needs a scoped Emscripten proof; desktop support alone is insufficient. Recheck upstream support/release practices. |

Box3D's C17 implementation is a useful fit with a C-first engine, but it does
not inherit Box2D's maturity automatically. Its documented WASM path is a reason
to test it, not evidence that Malvado's full export stack already works.
Jolt's [upstream WASM project](https://github.com/jrouwe/JoltPhysics.js) provides
portability evidence; Malvado should evaluate linking the native engine into
its own module rather than adding a separate JavaScript simulation world.

## Combined choice and shared boundary

Box2D plus Box3D could reduce language/toolchain diversity and make ID-based
world operations, events and build configuration easier to learn. This is an
architectural inference, not a common ABI or interchangeable shape guarantee.
Separate 2D and 3D adapters would still be necessary. A Box2D/Jolt or
Chipmunk/Bullet combination may be preferable if measured gameplay quality,
maintenance or platform coverage outweighs that convenience. Do not couple
2D delivery to the eventual 3D selection.

Keep the existing lightweight internal collision system in every configuration.
It is useful without simulation and has its own tested edge, ordering and
movement contracts. Share descriptors and conversions where valid; do not
reimplement an external rigid-body solver or route legacy overlap tests through
it by default. No need to support several interchangeable backends per dimension
until a real deployment requires it.

## Required selection evidence

Use the same acceptance workloads for competing candidates. Publish results
with revision, compiler, flags, hardware, body/contact counts, substeps and thread
count. Upstream scalability claims are not comparative Malvado measurements.

- Build headless core-only and optional-physics SDKs on Windows, Linux and macOS.
  Check installed static link dependencies, notices, allocator failure and
  language FFI. Confirm compiler standards without raising the base C core's
  requirements unnecessarily. Recheck tags, recent fixes, unresolved defects,
  API stability, contributor concentration and support channels.
- Test circles/boxes, resting stacks, sparse and dense scenes, sensors, fast
  objects and spawn/kill churn. Record step-time distributions, allocation
  counts, memory peaks, binary growth, build time and shape limits. Add character
  and static-mesh workloads when those capabilities enter scope.
- Test caller-driven fixed stepping and reproducibility with stable insertion
  order, compiler settings and threads. Box2D/Box3D advertise cross-platform
  determinism; Jolt documents its conditions. Do not extrapolate these claims
  to every build, engine version or Malvado event order. Physac, Chipmunk and
  Bullet also require measured repeatability. No library choice alone promises
  deterministic networking or save-state compatibility.
- Cross-build with the planned Emscripten SDK and test without pthreads first.
  Check SIMD fallback, supported memory/stack settings, floating-point behavior
  and exported size. Integrate browser regression tests with
  [web milestones](webassembly.md) before declaring web physics available.
- Measure the work to expose the selected capability through idiomatic C, Lua
  and experimental Rust over one native implementation. Recheck SDK target,
  precision and feature mismatch detection. No callbacks may unwind across FFI.

License labels above summarize upstream declarations. Review the pinned source,
subdirectories and required notices during selection. These permissive licenses
do not remove redistribution duties or ongoing dependency maintenance costs.
