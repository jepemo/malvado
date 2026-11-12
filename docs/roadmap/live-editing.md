# malvado-gui live editing, scenes and reload

Status: future planning only. No runtime, editor, transport, scene format or reload
API is implemented by this proposal. Milestones below are dependency units, not
assigned alpha or minor versions. Apply the [roadmap reevaluation rule](README.md#promotion-into-a-release)
before promoting each unit. The first 2.0 release gates remain unchanged.

## Current architecture and boundaries

The native core already provides context/process identities, snapshots,
properties, lifecycle operations and resource ownership. C, Lua and Rust share
that scheduler. The runner has owner-thread debug hooks, pause and tick-step
operations; these are not a general external editing service. Existing GUI
[instrumentation planning](../specs/gui.md#g2-process-instrumentation) describes
safe points and immutable snapshots. Reuse those contracts where practical.

Native image references, selected graphs, animation clocks and command lists
already have ownership contracts. They do not provide file watching or hot
replacement. Registered language process definitions are not yet a shared scene
factory registry. A JSON scene, authored identity and reconciler do not exist.
malvado-gui is the editor consumer in this proposal; the engine remains usable without it.

Keep process state authoritative in C. A property command must have the same
observable result for C, Lua and Rust processes. Bindings may translate their
idiomatic APIs and resource references; the inspector must not require a custom
implementation in every game process. Do not introduce an ECS, component model,
Godot-style node hierarchy or another scheduler for editing.

## State and identity boundaries

| State or identity | Authority | Intended rule |
| --- | --- | --- |
| Runtime process state | C runtime | Current position, render selection, status and other delivered engine properties. Changes go through validated operations at safe points. |
| Scene-authored state | Scene document | Initial properties and deliberate authored overrides; runtime movement does not rewrite it. |
| malvado-gui editor state | Editor | Selection, viewport, unsaved edits, undo history and play session; not game process ownership. |
| Scene instance ID | Scene document and its scope | Stable authored string such as player_01; uniqueness and scene namespace must be validated. |
| Runtime process identity | Runtime session plus native process reference | Refers to one actual process. Restart creates a new identity; old handles remain invalid. |
| Process definition key | Registered game definition | Names a factory/type, not a suspended execution state or a runtime PID. |

Maintain an explicit association between scene instance and current runtime
process; never coerce a scene string into a native PID. Dynamic instances remain
inspectable and use ordinary lifecycle semantics. They are not automatically
saved into a scene. A terminated managed instance can have no live association;
restarting it must be an explicit policy, not accidental resurrection on every
file event. Scene membership does not create a special process parent.

## E1: live built-in property editing

- **Goal:** inspect an actor and change its position while paused without restart.
- **Scope:** enumerate live processes and inspect identity/type/status plus common
  applicable properties: x/y/z, angle, size/scale, graph and existing selection
  fields. Explicitly distinguish writable properties from read-only identity,
  ownership and diagnostic data. Status changes use lifecycle operations.
- **Dependencies:** current snapshots, context identities, native setters and
  runner safe points. Investigate reusing the local opt-in diagnostics channel;
  transport, schema and public names remain undecided. Conceptual operations are
  list_processes, get_process, get_property and set_property.
- **Behavior:** malvado-gui sends a bounded request to the runtime owner thread. Pausing
  stops simulation ticks while control requests remain serviceable. Validate
  current session/liveness and return the accepted value or a useful error.
  Resource-valued graph edits resolve valid resources, not arbitrary pointers.
- **Acceptance:** edit position, angle, graph and scale on C/Lua/Rust actors;
  verify frozen/paused visibility and native readback, invalid/dead/foreign keys,
  readonly rejection and an unchanged process identity. Measure snapshot/command
  overhead with a useful process count; do not copy entire language heaps.
- **Non-goals:** arbitrary VM evaluation, raw Rust/C field access, remote service,
  code reload or a second object model.
- **Open questions:** running-mode conflict policy, expected revision/tick checks,
  batch validation, graph loading, camera/scroll projection and audio while paused.

Prefer a simple initial conflict rule: apply edits at a documented safe point;
subsequent game writes may overwrite them. Show that result in malvado-gui rather than
silently freezing or replacing game logic. Persistent editor overrides need a
separate explicit rule. Never access Lua or process memory from a UI/watch thread.

## E2: graphical asset hot reload

- **Goal:** save a sprite image and see referencing actors update without restart.
- **Scope:** start with a narrowly selected image path and explicit reload;
  evaluate watching/debouncing as a convenience over the same operation. Keep
  code reload independent. Add atlas/animation or other asset families separately.
- **Dependencies:** development asset provenance, current immutable image/graph
  ownership, renderer-safe replacement and diagnostics from E1 where useful.
- **Behavior:** load and validate a candidate before replacing references. Prefer
  stable logical asset identity while native backing objects may change. Current
  immutable resources must not silently acquire incompatible mutation semantics.
- **Acceptance:** multiple processes see a valid replacement and keep their PIDs;
  a malformed/partially written file leaves the previous asset visible. Test
  queued drawing, shared graphs, changed dimensions, repeated reload, final
  release, kill and context/window shutdown without stale references or leaks.
- **Non-goals:** all asset types, process recreation, code reload or a new generic
  resource framework merely to support one image case.
- **Open questions:** explicit reference rebinding versus a logical asset slot,
  normalized paths, packaged assets, update fan-out, removed files and animation
  clock preservation when metadata changes.

Review [graph collider rules](../api/collision.md) during implementation. Proposed
behavior: regenerate graph-derived geometry from the accepted visual, including
stable animation union bounds where applicable. Preserve explicitly overridden
colliders and collider-disabled mode. A local override may still follow the
process transform under its existing contract. Specify the exact safe point so
collision/world queries and rendered geometry do not disagree for a tick.

## E3: flat declarative scene loading and authoring

- **Goal:** open, edit and save a small level made of ordinary process instances.
- **Scope:** a readable versioned JSON document, registered definition lookup,
  initial properties and a scene-to-runtime identity association. Initially flat.
- **Dependencies:** E1 property validation and a minimal language-neutral factory
  registration contract. C/Lua/Rust factories may require binding adapters, but
  the document and built-in property semantics must not depend on the language.
- **Behavior:** validate definitions, IDs, properties and asset references before
  activation. Instantiate through existing process APIs. malvado-gui edits authored data
  and saves deliberately; dynamic instances stay outside the document.
- **Acceptance:** load an equivalent level with definitions implemented in each
  language; report duplicate IDs, unknown definitions, bad types and missing
  assets with document locations. Save/reopen deterministically. Verify partial
  load cleanup, attached/detached descendants, termination and handle invalidation.
- **Non-goals:** prefabs, nested scenes, a new scene graph, arbitrary field
  serialization or making the scene a lifetime owner automatically.
- **Open questions:** registration keys/versioning, allowed value types, property
  defaults, activation timing, scene unload/restart scope, explicit authored
  ownership relationships and whether E6 metadata is needed for a selected game.

Illustrative schema only; graph strings require future resource resolution:

```json
{
  "name": "Level 01",
  "instances": [
    {"id": "player_01", "process": "Player", "properties": {"x": 100, "y": 200}},
    {"id": "rock_01", "process": "Rock", "properties": {"x": 300, "y": 180, "graph": "rock.png"}}
  ]
}
```

## E4: scene reconciliation

- **Goal:** update authored level data without resetting unrelated game state.
- **Scope:** compare the last accepted and candidate authored documents by stable
  instance ID. Preserve unchanged processes and runtime state.
- **Dependencies:** E3 and explicit lifecycle/property update policies.
- **Behavior:** new instance spawns; removed instance terminates its associated
  managed process; changed authored property updates that process. Unchanged
  authored x=100 must not reset a player that moved to x=350. A changed process
  definition is a structural/code-reload decision, not a normal property write.
- **Acceptance:** add/remove/edit an obstacle while a player keeps moving; check
  unchanged PIDs/state, descendant ownership, stale-handle rejection, failed
  validation and repeated application without duplicates. Saving must not capture
  gameplay movement unless the user explicitly applies selected runtime values.
- **Non-goals:** transparent rollback of game side effects, process-code migration,
  automatic persistence of dynamic processes or first-version undo infrastructure.
- **Open questions:** property removal/default restoration, edit/play overrides,
  user save/apply actions, dead managed instances, type replacement and undo/redo.

Validate the complete change set and stage resources where practical. Existing
spawn/kill operations do not establish a multi-process transaction. Define which
operations can roll back, whether partial commit is possible, and how malvado-gui reports
and recovers from it. Do not promise to undo I/O or destructor side effects.
Runtime-created children retain ordinary ownership: attached children may die
with their owner; detached children must not be removed merely because a scene
association changes. Define restart/unload handling to avoid duplicated survivors.

## E5: selective Lua code reload

- **Goal:** revise a process behavior while unrelated actors keep running.
- **Scope:** detect/request a script change, validate candidate definitions, update
  registration and selectively restart affected instances. Preserve scene-authored
  inputs during recreation; do not promise preservation of coroutine locals.
- **Dependencies:** definition registry, E3 identity mapping and explicit restart
  policy. E4 is useful for editor integration but not required for definition-only
  reload affecting future spawns.
- **Behavior:** a new function does not replace a suspended coroutine body.
  Prefer definition replacement plus explicit selective restart initially;
  future-instance-only replacement is a simpler alternative to evaluate.
- **Acceptance:** valid code changes affected behavior with fresh runtime handles;
  unrelated processes keep state. Syntax/registration errors keep old working
  definitions and instances. Test repeated reload and owned/detached children.
- **Non-goals:** transparent stack/local migration, arbitrary module side-effect
  rollback, Rust reload or stopping every process for a local change.
- **Open questions:** dependency tracking, dynamic instances of affected types,
  shared modules, opt-out rules, restart sequencing and candidate isolation.

Separate definition validation from registration commit and destructive restart.
Executing candidate Lua can have side effects; define a restricted registration
contract or state the rollback boundary. A failure after old instances terminate
cannot resurrect their original coroutines. Report such failures accurately and
keep the accepted definition revision and scene/runtime association consistent.

## E6: custom property descriptors

- **Goal:** inspect and edit game-specific values with little boilerplate.
- **Scope:** evaluate lightweight typed metadata with defaults, readonly/editable
  flags, validation and optional editor presentation. Start with a small type set.
- **Dependencies:** E1; promote earlier if selected scene properties need it.
- **Behavior:** a Lua definition might use defaults such as
  `process(run, {defaults={speed=2, health=100, damage=10}})`. This illustrates game
  fields, not an existing cross-language reflection guarantee. C and Rust need
  idiomatic safe registration/access mechanisms, not arbitrary memory reads.
- **Acceptance:** malvado-gui shows and edits equivalent speed/health properties in
  C/Lua/Rust; invalid writes preserve values and readonly fields reject edits.
  Distinguish defaults, authored overrides and current runtime values.
- **Non-goals:** serializing arbitrary language objects, executing getters to
  populate every snapshot or mandatory annotations on all internal game state.
- **Open questions:** supported scalars/resources, metadata lifetime, safe-point
  accessors, validation cost and explicit opt-in for non-engine state. Custom
  state may remain language-owned; its exposed editor view uses a shared contract.

## E7: Rust rebuild/restart and advanced code reload

- **Goal:** improve Rust iteration while preserving editor context safely.
- **Scope:** first evaluate compile diagnostics plus game restart with malvado-gui scene,
  selection and authored data retained. Dynamic-library reload is optional research.
- **Dependencies:** existing Rust CLI build/run and E3/E4 mappings where used.
- **Behavior:** recompilation may recreate the game session; old runtime handles
  invalidate. Map scene selections back by authored ID rather than reusing PIDs.
- **Acceptance:** compile errors leave useful diagnostics and the previous usable
  session where feasible; a successful restart restores authored scene inputs and
  editor context with a new runtime identity. Verify supported host toolchains.
- **Non-goals:** arbitrary replacement of suspended Rust futures, layout-preserving
  magic, or dynamic modules as a prerequisite for a useful malvado-gui editor.
- **Open questions:** build cost, restart scope, optional explicit state snapshots;
  for later native modules, stable C ABI, allocator/destructor ownership, live code
  pointers, module unloading, type/layout changes, panic boundaries and platforms.

Dynamic native modules or compatible definition swapping require a separate
safety proof. C code reload has similar native-code risks and is not implicitly
provided by Lua reload or language-independent property editing.

## Later research: reusable scenes and prefabs

Evaluate reusable flat groups, explicit per-instance overrides, shared definitions
and update propagation only after E3/E4 have real game evidence. Nested references
need cycle, identity, ownership, resource-path and override rules. Promotion of a
dynamic process into authored data must be an explicit user operation. Neither
capability selects a prefab schema or introduces a node hierarchy today.

## Scheduling and first useful malvado-gui editor

E1 and E2 can deliver independent useful development tools. E3 adds authored
levels; E4 adds incremental editing. Together these form a useful first live scene
editor without code reload. E6 can move ahead of E3 for a concrete inexpensive
property need. E5 and Rust rebuild/restart can follow independently; advanced
native reload and prefabs remain long-term research.

Group or split these units into small future feature alphas only when scheduled.
There is no one-phase/one-version requirement and no additional 2.0 release gate.
All promoted work needs C/Lua/Rust behavior where applicable, lifecycle/resource
checks and relevant supported-platform automation. Later effects, shader reload
and spatial attachment inspectors consume these foundations; they do not block
basic process editing or flat scenes.
