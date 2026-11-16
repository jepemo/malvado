# Optional physics for processes

Status: architectural exploration, researched on 2026-11-16. No physics API,
backend or release number is approved. Apply the
[roadmap promotion rule](README.md#promotion-into-a-release) before implementation.
This proposal changes no current collision or process behavior.

## Goal and current architecture

Physics should serve Malvado's processes. A process remains the game object;
physics is an optional capability, with automatic synchronization and cleanup.
Games must remain usable without a physics dependency. Do not add an ECS-facing
API, a second process scheduler or mandatory body management to simple games.

The source review found these foundations:

- `src/core/collision.c` and `src/include/malvado/collision.h` provide rectangle
  tests, transformed circles/rectangles, and static rectangle-world queries,
  sweeps and bounded movement. Movement stops at impact; it does not slide or
  integrate forces. Process queries use creation order and include sleeping and
  frozen processes. Edge contact is not overlap. These are useful delivered
  behaviors, not an incomplete rigid-body solver.
- `src/core/process.c` owns native process transforms, identity, scheduling and
  safe-point cleanup. Selected colliders can come from a primitive graph, a
  bound local override or an independently registered fixed transform.
- `bindings/lua/runtime.c` resolves Lua fields and image/atlas-derived shapes
  into the same native collision registry. Its field synchronization must be
  reviewed before physics writes transforms; writing native values alone could
  leave Lua properties or derived bounds stale.
- `src/core/clock.c` implements fixed ticks with bounded catch-up and records
  dropped time. `mv_context_advance` determines tick counts; process cadence is
  separate from render frequency. Physics must use this timing, not wall time
  or a library's autonomous update loop.
- Lifecycle ownership, creator identity and proposed spatial attachments are
  distinct. Generic resource references can outlive their process or context;
  retaining a wrapper must not keep a dead process simulating.
- `src/platform/raylib.c` renders current 2D content. Modes 7/8/9 are proposals,
  not available simulation spaces. Physics math and tests must work headlessly.

See the [collision contract](../api/collision.md),
[process inventory](../api/process-inventory.md),
[cleanup rules](../api/process-cleanup.md),
[resource lifetime](../api/resource-lifetime.md), and
[multi-language architecture](../architecture/multi-language.md).
Some older design pages still describe completed alpha work as open; consult
current implementation and version evidence when promoting this proposal.

## Keep detection and simulation separate

Retain lightweight overlap, hitbox and static-world movement APIs without a
physics world. Asteroids, arcade and scripted platform games often need only
those operations. A rigid-body world adds integration, mass, contact response,
friction, gravity and constraints. It must not silently enroll every collider.

Reuse shape descriptors, local pivots, coordinate conversions and validation
where semantics match. An initial physics shape can be copied from the selected
process collider. Do not duplicate algorithms in Lua or Rust. Do not replace
existing exact overlap semantics with backend contact tolerances: resting bodies
can have solver contacts without a strict overlap. Explain the distinction in
one collision guide. Existing process queries retain their ordering and results.

Simulation owns its own broad phase and contacts. Native gameplay queries may
still use the small collision registry. Synchronize transforms at documented
boundaries; do not run two response solvers on one process. In particular,
`world:move` must not silently compete with a dynamic body's integration. P1
must choose rejection or an explicit supported kinematic operation for this case.
Static collision worlds do not automatically become physics terrain; an explicit
conversion may copy rectangles, with ownership and later-update rules defined.
Physics filters do not retroactively change existing process type queries.

## Library evaluation

The [candidate comparison](physics-libraries.md) covers Physac, Box2D,
Chipmunk2D, Box3D, Jolt and Bullet. No library is selected. Box2D and Box3D are
promising C-first candidates, but shared authorship does not establish equal
maturity or a common ABI. Compare pinned releases and real workloads first.

Start with one optional integration module per delivered dimension, with a
private backend adapter and backend-neutral public values. Do not build a
runtime plugin framework or a lowest-common-denominator backend interface.
CMake options should omit physics entirely, including dependencies, when unused.
A C17 dependency may have its own compilation standard while Malvado stays C11.
C++ candidates require a private C boundary and optional C++ build/runtime support.
No engine-library types should escape through the C SDK.

Use the same C behavior from Lua and experimental Rust. C should use conventional
configuration structs, checked results and process references. Lua should favor
small global operations, with advanced world/joint operations under an optional
namespace. Rust should use typed configuration, safe references and RAII over
native ownership, within its owned-future process model. Dropping a view and
ending a process must not cause double destruction. Differences in language
availability must be explicit; no example is aligned until all three run.

## Physics needs by visual mode

The table proposes gameplay choices, not new rendering contracts. A camera or
isometric projection never determines the simulation dimension by itself.

| Visual/gameplay case | Suitable simulation and shapes | Movement, coordinates and delivery |
| --- | --- | --- |
| Current 2D arcade / Asteroids | No physics required; optional 2D circles, boxes and later convex pieces | Zero gravity for space/top-down games. Wrapping and arcade steering remain gameplay rules. P2 supports optional bodies. |
| 2D platformer | Scripted sweeps or 2D rigid bodies; boxes/circles first, capsules later | Downward screen gravity, locked character rotation where appropriate; ground normals, slopes and moving platforms need a controller, not just a dynamic box. P2 basic objects; P3 controller. |
| Top-down or isometric | Usually 2D on a ground plane | Zero gravity unless the game explicitly models height. Isometric screen coordinates are a projection of gameplay coordinates. P2 after an explicit mapping; no automatic 3D. |
| Mode 7 proposal | Usually 2D floor-plane circles/boxes; optional scripted height | Perspective floor and sprites do not require volume physics. Define floor units and project world positions before rendering; gravity is off or a separate gameplay height rule. Adapt after the Mode 7 world transform exists. |
| Mode 8 proposal | 2.5D sector movement, with optional true 3D later | Sector footprints, floor/ceiling heights, portals, stairs and slopes require explicit traversal and collision geometry. A 2D footprint alone cannot certify vertical clearance. Reuse basic mode collision first; P4 only if volume interactions justify it. |
| REPLACED-like 2.5D presentation | 2D side-view simulation can drive sprites/objects in a 3D scene | One gameplay plane, downward gravity and controlled depth lanes if needed. Depth for visuals does not imply 3D response. P2/P3 concepts plus a delivered scene projection. |
| Mode 9 proposal, true 3D gameplay | 3D boxes/spheres, then capsules/convex shapes; static meshes later | World gravity follows the agreed scene axes; dynamic bodies drive scene transforms, cameras only observe. P4 requires mature scene ownership/coordinates. P3-style controller requirements must be evaluated separately in 3D. |

Keep current process `x/y` in pixels, clockwise `angle` in degrees and `z` as
2D draw order. Propose an explicit pixels-per-physics-unit conversion for 2D,
with documented linear/angular velocity and force units. Use positive screen Y
for default downward 2D gravity; top-down games need a simple zero-gravity choice.
The default magnitude and scale need measured examples before approval. Convert
at the native boundary, not per language. Future 3D scene transforms are separate
from `self.z`; reconcile with the graphics proposal's right-handed Y-up space.
Never infer 3D collision geometry from the projected sprite or camera.

## Exploratory Lua experience

These snippets are pseudocode for discussion. `self.physics` and the physics
functions below do not exist. Final field names, options, defaults and return
values must be reevaluated against the API available during implementation.

```lua
-- Proposed only: attach physics to the process, then keep it alive.
local Ball = process(function(self)
    self.collider = collision.circle(8)
    self.physics = { bounce = 0.8 }
    while true do frame() end
end)
```

The intended default is a dynamic body in a lazily created context-local 2D
world, with gravity and a supported selected collider. A supported graph could
supply the shape instead of the explicit collider. No mandatory world creation,
body handle or per-frame synchronization should be needed for this case.
A process that returns immediately must release its body, just as other
process-owned state is released.

```lua
-- Proposed only: static terrain and an explicit shape override.
self.collider = collision.rect(160, 16)
self.physics = { body = "static", friction = 0.5 }

-- Proposed only, inside a live dynamic process:
impulse(0, -120)
-- Advanced operations could live under physics, for example:
physics.teleport(self, 40, 80)
```

Keep one primary shape configuration path. Prefer the selected collider over a
second independent `physics.shape` description. If a shortcut such as
`shape = "circle"` proves useful, normalize it into that same descriptor with
explicit sizing rules. Unknown or ambiguous graphics should fail at activation,
not create a zero-size body. `collider = false` plus physics is a configuration
error unless a separate explicit physics shape was intentionally approved.

Shape precedence to evaluate: explicit local collider, then supported graph or
image bounds with its documented pivot. Preserve trim, atlas/FPG pivots, rotation
and scale exactly once. No pixel masks, automatic concave tracing or GPU readback.
Fixed world-space collider registration needs an explicit conversion or rejection;
it cannot silently become local geometry. Copy the initial shape; animation,
graph or scale changes should not rebuild mass/contact state every frame.
Propose an explicit refresh operation, validated and committed atomically.

For dynamic bodies, propose density with a useful default and optional explicit
mass override; reject conflicting mass/density settings until precedence is
specified. Define inertia, finite positive values, material mixing, friction
and restitution ranges and invalid-input rollback. Static and kinematic bodies
must not acquire mass from visual size. Common controls include gravity scale,
linear/angular velocity, fixed rotation, forces and impulses. Sensors detect
contacts without response; category/mask layers are distinct from process type.
Detailed numerical defaults and maximum layer counts remain P1 decisions.

## Transform authority and tick ordering

Proposed authority rules require a contract review; they do not change today's
property writes:

- Without physics, process transforms and collision behavior remain unchanged.
- Dynamic bodies own simulated position/angle after stepping. Prefer an explicit
  teleport operation that updates native state, Lua-visible values and collision
  bounds together. Reject direct position/angle writes while dynamic physics is
  active, rather than silently resetting velocity. Reevaluate this choice against
  a convenient property-to-teleport alternative in P1; never use both implicitly.
- Kinematic processes supply target motion for the next fixed step; the adapter
  computes a meaningful velocity. Define targets, teleports and obstruction
  behavior separately. Static moves require an explicit safe-point update.
- Physics writeback must bypass command setters internally to avoid a feedback
  loop. Rendering interpolation, if added, is visual only; queries read committed
  simulation state. Cameras consume that state after writeback.

Proposed tick sequence: expose prior-step events; execute the existing process
tick in its established priority/order; remove ended processes and commit queued
body commands; step physics with that fixed tick duration; write back live
transforms and refresh query bounds; copy contact events for the next tick.
Validate the insertion point against native safe-point cleanup and Lua field
synchronization before approval. Define spawn visibility, first-tick bodies,
force duration and command order explicitly. A force is accumulated for one
physics tick; an impulse is a one-time velocity change. Neither depends on FPS.

Every simulation tick runs even if a process's `frame` cadence skips user code.
Physics substeps are bounded solver work inside that tick, not process ticks.
Reuse catch-up limits and dropped-time accounting; do not advance from render
callbacks independently. Fixed stepping helps reproducibility but does not
promise bitwise results across compilers, CPUs, browsers or backend versions.
Specify repeatability within a pinned build first and test cross-platform claims.
No networking, lockstep or rollback system is included.

## Lifecycle, signals and events

Propose one lazily created world per context and dimension initially, destroyed
after bodies and pending operations. No global singleton world. Multiple worlds,
world transfer and cross-world joints are deferred until a concrete game needs
them. World gravity and scale need simple game-level defaults with optional
advanced configuration; graphical windows are not world owners.

Body activation validates geometry and allocates atomically; failure leaves the
process usable without a partial body. Bodies are tied to live process references
and runtime identity. Return, error, kill and owner cascades disable simulation
participation immediately, then release backend storage at a safe point.
Detached children keep their own bodies while alive. Creator relationships do
not imply physical ownership. Killing an owner must preserve existing process
cleanup order; constraints referencing dead bodies must be invalidated before
those bodies are freed. A retained body wrapper can expose dead identity but
cannot keep an ended process active or access destroyed world storage.

Never execute Lua, Rust futures or process callbacks inside a backend solver
callback or worker thread. Copy events with validated process references, normal,
point and phase into a bounded native queue. Deliver begin/end contacts and
sensor transitions on the owner thread at the next safe process opportunity;
resolve liveness again before each delivery. Killing another process while
handling an event is safe. Specify whether end events survive body removal,
queue overflow behavior, deduplication across substeps, ordering and stale-peer
identity before exposing events. Sleeping process queues must be bounded.

Distinguish scheduler sleep/freeze from backend body sleep. Proposed default:
a sleeping process's body continues simulating; a frozen process's body pauses
with velocities saved and resumes explicitly. P1 must decide how a frozen body
blocks others and what events it receives, including wake behavior. Preserve
existing collision-only participation for both states.

Lifecycle attachments never imply transform inheritance or joints. Visual
attachments may follow a physics process after writeback. Reject a dynamic body
with a conflicting inherited spatial transform until an explicit policy exists.
Constraints/joints are separate optional resources, owned and invalidated
explicitly; parent/child relationships alone create none.

## Future milestones, without release numbers

All phases follow the existing 2.0 release path. They do not modify alpha.8,
beta or the tentative web export slots. Basic 2D work is independent of future
rendering modes and editor delivery. Each promoted phase needs bounded tasks,
automated evidence and explicit C/Lua/experimental Rust availability.

### P1: collision boundary and backend proof

- [ ] Audit delivered collision/transform contracts against C/Lua/Rust, including
  fixed versus bound colliders, derived image bounds and current numeric limits.
  Preserve regression coverage for edge contact, ordering and world movement.
  Resolve simulation authority, tick insertion, units, signals and event policy.
- [ ] Compare pinned candidate releases with a bounded headless workload: falling
  ball, resting stack, sensor, fast mover and repeated spawn/kill. Record compile
  cost, binary size, allocations, step time and repeatability. Select a backend
  only after Windows/Linux/macOS build evidence, license review and a documented
  Emscripten feasibility result. This task performs no such builds or benchmarks.
- [ ] Record optional build/SDK contracts and unavailable capabilities. Ensure
  the existing core stays usable without Lua, graphics, physics or a C++ toolchain.

### P2: useful 2D simulation integrated with processes

P1 is required. Lifecycle and synchronization are essential here, not a later
polish phase. Excludes joints, complex characters and 3D.

- [ ] Deliver optional static/dynamic/kinematic bodies with circles/boxes,
  gravity, mass/density rules, friction, restitution, velocity, forces/impulses,
  filters and sensors. Add safe queued contact events and headless debug-shape
  data. Validate invalid inputs, activation failure and numeric boundaries.
- [ ] Verify automatic transforms and cleanup for return/error/kill, attached
  and detached lifetimes, signals, event-time kill, stale/foreign references,
  allocation failure and context shutdown. Long spawn/kill runs must not leak.
- [ ] Compare traces across render rates and process cadences. Run existing
  collision suites unchanged with physics off and on; prove physics-disabled
  builds and installed SDK consumers still work. C, Lua and experimental Rust
  expose the same accepted concepts or explicitly record incomplete support.
- [ ] Deliver aligned falling-ball and top-down examples with shared assets,
  compilation, bounded runs and pixel/input checks. Debug outlines must match
  colliders under camera motion, pivot, rotation and scale. Mark missing variants
  incomplete. Run native platform CI; when W1 exists, add single-threaded WASM
  simulation tests and require browser checks before claiming web physics.

### P3: selected 2D gameplay helpers and constraints

P2 is required. Select features from real platformer or vehicle examples rather
than exposing every backend option.

- [ ] Design ground queries from contact normals, slope limits and a configurable
  up direction. Choose a kinematic controller or dynamic-body helper explicitly.
  Test stairs, one-way/moving platforms, tunneling and initial penetration for
  each claimed capability; existing `world:move` remains an independent option.
- [ ] Add justified convex/compound shapes, continuous collision settings and a
  small joint set. Test endpoint death, joint ownership, force limits and cleanup.
  Collision filtering, sensors and joint behavior must match across languages.
- [ ] Validate aligned gameplay examples and performance budgets at measured
  body/contact counts. Record shape/material limitations and deferred helpers.

### P4: bounded true 3D simulation

Requires P1 lessons plus mature Mode 9 scene transforms, resource ownership and
static collision geometry. Does not require every P3 feature or force Mode 7/8
to use 3D. Reevaluate Box3D maturity against Jolt/Bullet when this phase starts.

- [ ] Select a 3D backend after the same platform, license, performance and
  maintenance review. Define 3D units, orientation, scene/body synchronization
  and lifecycle without repurposing 2D `z`. Preserve headless and no-physics builds.
- [ ] Deliver boxes/spheres, static/dynamic/kinematic bodies, contact response,
  filters, sensors and safe events. Compare native/language lifecycle traces;
  verify scene depth and camera-independent debug geometry visually.
- [ ] Add capsules/convex bodies and static triangle meshes only with valid-shape,
  terrain, memory and cleanup fixtures. Character controllers, sector bridges,
  joints and dynamic concave bodies require separate selected acceptance tasks.
  Record unsupported modes/platforms; run web tests before claiming WASM support.

### P5: optional authoring and advanced integration

Depends on stable P2 or P4 contracts and the corresponding
[editor milestones](live-editing.md). No editor prerequisite for physics itself.

- [ ] Expose copied inspection/debug data and scene configuration after scene
  schemas exist. Native simulation remains authoritative. Authoring previews
  must use the same physics operations as games.
- [ ] Define pause/edit/rebuild transactions for shape or material changes and
  hot reload; stale commands/events cannot reach replacement bodies. Test invalid
  edits, rollback, ownership and restart. Do not promise restoration of arbitrary
  solver state or deterministic replay from scene data alone.

Advanced ragdolls, cloth, soft bodies, fluids, vehicles, multiple worlds,
threaded simulation and rollback remain outside these phase requirements.
Manual device and complete gameplay checks belong to each future release's beta;
they supplement automated simulation, graphics and input checks.

## Decisions to revisit before implementation

P1 must resolve body activation syntax, shape precedence/refresh, world defaults,
scale/units, material and mass rules, transform authority, fixed-step insertion,
signals, event payload/order/overflow and cleanup transactions. P2 must confirm
language availability, optional SDK feature detection and ABI mismatch handling,
debug output and measured resource budgets. P3 must select controller semantics
and each joint/shape feature independently. P4 must recheck backend maturity,
3D scene coordinates, static geometry and browser toolchains. P5 must reconcile
scene identity, editor commands and reload behavior with native ownership.

These are review gates, not permission to alter today's APIs. If a phase needs
an API change, record its reason, update migration guidance and tests, and keep
native SDK mismatch rejection correct before marking that phase complete.
