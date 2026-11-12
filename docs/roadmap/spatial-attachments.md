# Process spatial attachments

Status: future proposal, with no release assigned. Apply the explicit
[reevaluation rule](README.md#promotion-into-a-release) before implementation.
The API sketches are provisional and are not delivered process fields.

## Existing process model

The [process contract](../api/process-inventory.md) separates creator identity
from lifecycle ownership. Native related spawn and genealogy operations already
support attached/independent lifetimes. Rust attached/detached spawn and Lua
process creation use the same scheduler. Current 2D position, angle and scale
are process values; graph pivots, animation frames and collider transforms have
existing contracts. FPG point data and atlas metadata do not implement spatial
links or a named world-point query.

Lifecycle ownership must not imply transform inheritance. A player's bullet may
have an owner and move independently; a hat may follow a head point; an owned
effect may remain in world space. Keep creator, lifetime owner and spatial target
as distinct concepts. Do not introduce a second ownership hierarchy or scheduler.

## S1: explicit relative 2D transforms

- **Goal:** a hat follows a moving and rotating actor without copying coordinates
  in each coroutine.
- **Scope:** optional process-to-process spatial links, local translation,
  rotation and scale, with explicit attach/change/detach behavior.
- **Sketch:** `spawn(Hat, {parent=self, attach_to="origin", offset={x=0,y=-4}})`.
  `parent` means lifecycle ownership in this sketch; confirm actual spawn naming
  before implementing it. No existing Lua spawn option is added by this document.
- **Dependencies:** review process properties, visual projection and collision
  update timing against the current C/Lua/Rust contracts.
- **Acceptance:** movement, rotation and scale compose correctly; a bullet owned
  by the same actor remains independent. Test cycles, chains, sleep/freeze,
  cleanup and deterministic results independent of process execution priority.
- **Deferred:** named metadata points, 3D and scene-graph redesign.
- **Questions:** attach through spawn or one minimal property/operation; whether
  spatial targets may differ from lifecycle owners; local/world read/write rules;
  inheritance defaults and preserving world position on detach/reparent.

Prefer a single obvious configuration path. Decide if all transform components
inherit by default before adding per-component switches. Do not overload the
existing `z` draw-order field with spatial depth.

## S2: named and animated sprite points

- **Goal:** a weapon follows a named hand and a projectile can query its world
  origin without being spatially linked.
- **Scope:** named asset points, world-point queries and current-frame resolution.
- **Sketch:** `attach_to="right_hand"`; a possible query is
  `local x,y = self:point_world("right_hand")`. Query naming is unresolved;
  compare historical `GET_REAL_POINT` and existing numbered FPG points.
- **Dependencies:** S1, asset metadata and authoritative native animation sampling.
- **Acceptance:** a weapon follows a moving/rotating sprite; changing animation
  frames changes its point correctly. Verify trimmed sprites, fractional pivots,
  missing points and graph replacement using shared C/Lua/Rust fixtures.
- **Deferred:** a new binary format, full historical point compatibility and editor.
- **Questions:** metadata schema and name matching, numbered-point mapping,
  `origin` reservation, frame fallback, shared immutable point data and whether
  the independent query should be delivered before the linking feature.

Examples of names are origin, head, left_hand and right_hand. Reuse the existing
PNG/JSON/atlas architecture where useful. Do not require an editor to author the
first runtime fixtures. Define transformations relative to cropped/uncropped
coordinates explicitly; avoid applying trim and pivot adjustments twice.

## Relationship and timing decisions

Resolve these cases before either milestone becomes a public contract:

| Case | Preferred direction to evaluate and required evidence |
| --- | --- |
| Owner/creator differs from spatial target | Permit only if a clear independent target reference is safe; test that spatial linking never silently transfers lifetime ownership. |
| Target dies | Lifecycle rules decide whether the child dies. A surviving child could detach at its last world transform; specify notification/error behavior and test stale references. |
| Child dies | Remove its spatial link and owned data once without affecting target or siblings. |
| Detached lifecycle child | It remains lifecycle-independent even if it has a spatial link. |
| Change target, point or offset | Validate atomically; failed changes preserve the previous link. Decide preserve-world versus preserve-local semantics. |
| Target has no graph or lacks a point | An explicit transform origin may work without graphics; named unavailable points need useful errors, not an origin fallback hidden from the caller. |
| Graph/mesh/frame changes | Resolve against current native visual data. Define missing-point policy and avoid retaining stale frame pointers. |
| Nested links or cycles | Validate self/ancestor cycles and cross-context targets. Resolve a bounded acyclic dependency order; no recursive process execution. |
| Sleeping or frozen target/child | Specify transform resolution separately from execution and visibility. Test a frozen accessory following an active target and an invisible target. |
| Update order | Propose resolve after process writes and animation selection, before rendering. Decide how collision/movement queries obtain current world transforms without a one-tick lag. |
| Rendering order | Keep draw order independent of spatial links unless a separate explicit layer rule is justified. |

All directions in this table remain proposals. The existing lifetime rules stay
authoritative. Inspect current safe-point cleanup and frozen-process behavior
again during implementation rather than copying this table into runtime code.

## S3: basic Mode 9 targets

- **Goal:** a 3D accessory follows a named model attachment target.
- **Scope:** simple model nodes/points and relative transforms after Mode 9 exists.
- **Sketch:** `spawn(Hat, {parent=self, attach_to="Head"})`, with a model target.
- **Dependencies:** S1/S2 concepts and implemented model/world-transform conventions.
- **Acceptance:** a moving/rotating model carries an accessory, and replacement,
  target loss and invalid nodes follow documented rules across the three APIs.
- **Deferred:** skeletal animation, bone sockets and a complete 3D scene graph.
- **Questions:** coordinate units, rotation representation, nonuniform scale,
  supported model formats, graph billboards and point/node namespace rules.

## S4: skeletal sockets and editor consumers

- **Goal:** expose useful existing animation targets and visual authoring tools.
- **Scope:** separate later evaluations for bone/socket attachment and editor UI.
- **Sketch:** retain the named-target concept where semantics permit; no new
  skeleton or editor API is selected here.
- **Dependencies:** implemented skeletal animation for sockets; runtime point
  metadata and the actual graphical workshop for editing.
- **Acceptance:** animated bones carry accessories correctly; an editor can name
  points, edit frame offsets, choose target/point, preview links and inspect world
  transforms without changing ownership or requiring different runtime semantics.
- **Deferred:** building skeletons, a full editor or hot reload for this proposal.
- **Questions:** node/bone distinction, animation blending, point persistence and
  undo/save semantics in the eventual editor.

C uses explicit identities, copied transforms and checked results. Lua property
or spawn syntax must not dictate C layouts or Rust ownership. Reuse the existing
scheduler, context identity, resource cleanup and platform acceptance practices.
