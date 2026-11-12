# Visual properties, effects and shaders

Status: future proposal. Nothing on this page is implemented or assigned a
release. The [roadmap promotion rule](README.md#promotion-into-a-release) applies:
reevaluate the API and technical design before implementation. Prefer a simpler,
consistent solution when actual architecture and game needs justify it.

## Current boundary

The engine has shared process graphs, ordered drawing commands, cameras, clips,
scroll and retained image resources. It has no public effect chain, shader
resource, camera render-target chain or Mode 9 material system. Existing primitive
colors are not a process tint API. No editor is required for these proposals.
All sketches below are provisional Lua, not runnable current examples.

## V1: fundamental visual properties

- **Goal:** change object opacity, tint and blending during play.
- **Scope:** direct process properties, initially for supported 2D graphs.
- **Sketch:** `self.alpha = 0.8; self.tint = "#ff8800"; self.blend = "add"`.
  Proposed defaults are 1.0, white and `"alpha"`; consider alpha/add/multiply.
- **Dependencies:** shared drawing and resource lifetime; define color parsing,
  numeric ranges, invalid values and applicable graph/command representations.
- **Acceptance:** an actor changes tint/opacity without changing its shared image;
  another actor using that image stays unchanged. Test blend pixels, ordering,
  cleanup and errors across C/Lua/Rust and supported backends.
- **Deferred:** effects, lighting, a complete material API and camera composition.
- **Questions:** color space, premultiplied alpha, propagation to commands, and
  Mode 9 interaction with transparent sorting, lighting and material overrides.

These properties describe fundamental object appearance. Avoid duplicate
`fx.alpha` or `fx.tint` operations without a demonstrated semantic need.

## V2: a small process-effect set

- **Goal:** add a useful visual transformation without exposing backend stages.
- **Scope:** select a small measured set, such as grayscale and dissolve.
- **Sketch:** `self.effects = { fx.grayscale(0.8), fx.dissolve(0.3) }`.
- **Dependencies:** V1, effect capabilities, owner-thread resource cleanup and a
  renderer experiment that measures batching, bounds and intermediate storage.
- **Acceptance:** grayscale works on an actor; two actors share immutable effect
  code while using independent amounts. Test invalid combinations, kill/clear,
  rendering order, transparent edges and errors without changing collision shape.
- **Deferred:** implementing every candidate, custom shaders and camera effects.
- **Questions:** ordered chains versus a simpler abstraction; list replacement;
  out-of-bounds effects; whether compatible operations can fuse without changing
  declared order; which effects require geometry or multiple passes.

Additional candidates are brightness, contrast, outline, wave/distortion, palette
remapping, dithering, blur and pixelation. Inclusion here is not approval.
Do not assume one fragment shader or one render pass per effect.

A mutable sketch is `local d = fx.dissolve(0); self.effects = {d}; d.amount = 0.5`.
Resolve the sharing ambiguity before implementation: immutable definitions may
be shared, while process parameter instances should normally be independent.
Decide whether assignment copies an instance or explicitly shares it. Specify
validation, snapshots, cleanup and editor-readable parameter metadata. Parameter
updates must not compile another shader unnecessarily.

## V3: custom shader resources and parameters

- **Goal:** implement a game-specific effect with useful diagnostics.
- **Scope:** a bounded custom-effect contract, not arbitrary pipeline access.
- **Sketch:** `local h = fx.shader("effects/hologram.frag", {distortion=0.03, scanlines=0.5, speed=2}); self.effects={h}; h.distortion=0.1`.
- **Dependencies:** V2, backend capability investigation and parameter schemas.
  The filename is illustrative; neither language nor representation is selected.
- **Acceptance:** two actors share compiled code but update independent parameters;
  malformed source and wrong parameter types produce actionable errors. Test
  partial-load failure, source release, process death and backend incompatibility.
- **Deferred:** unrestricted shader stages, a shader language, hot reload and IDE.
- **Questions:** GLSL versus other representations, portable profiles, compiler
  diagnostics, uniform types/defaults, texture inputs, time sources, caching,
  resource limits and a dedicated effect entry contract. Investigate before
  choosing GLSL, SPIR-V or a custom language.

## V4: camera output effects

- **Goal:** transform a complete camera output independently of object effects.
- **Scope:** one camera render target and a small post-processing experiment.
- **Sketch:** `camera.effects = { fx.pixelate(320,180), fx.dither(0.4), fx.vignette(0.3) }`.
- **Dependencies:** explicit render targets, pass lifetime and viewport composition.
- **Acceptance:** a camera's output is processed once; a selected HUD layer stays
  crisp. Test resize, nested scopes, transparent output and target cleanup.
- **Deferred:** bloom, grading and arbitrary multi-camera pass graphs until needed.
- **Questions:** regions/scroll, UI participation, intermediate texture ownership,
  multiple cameras, pass ordering and eventual Mode 9 depth buffers.

## V5: Mode 9 adaptation

- **Goal:** reuse effect concepts on appropriate 3D representations.
- **Scope:** adapt one compatible effect after Mode 9 foundations exist.
- **Sketch:** a sprite uses `self.graph` and a model uses `self.mesh`; either may
  select `self.effects = {fx.dissolve(0.5)}`. Resource strings are illustrative.
- **Dependencies:** model drawing, depth/transparency contracts and material rules.
- **Acceptance:** a dissolve works on both representations with clear capability
  errors for unsupported effects. Preserve depth, ordering and resource cleanup.
- **Deferred:** complete materials, lighting and skeletal animation just for effects.
- **Questions:** graph/mesh as mutually exclusive primary visuals; graph billboards;
  `self.material` as an optional surface override; interaction of alpha, lighting
  and surface shaders with additional effects. Materials and effects have distinct
  roles; no first 2D milestone requires a material framework.

## V6: reload and editor consumers

- **Goal:** edit parameters and shader source while inspecting a running game.
- **Scope:** evaluate shader reload independently of process-code reload.
- **Sketch:** existing effect properties remain editable; no reload call is fixed.
- **Dependencies:** stable runtime contracts, compiler diagnostics and an actual
  editor/inspection use case.
- **Acceptance:** compatible parameter values survive successful recompilation;
  failed recompilation reports errors and preserves the last usable effect.
- **Deferred:** Lua/Rust code reload and a general editor framework.
- **Questions:** file watching versus explicit reload, synchronization, compatible
  schemas and inspection of process/camera effect ownership.

C will use explicit handles, typed parameters and results. Rust will use safe
owned resources and idiomatic configuration. Neither binding must emulate Lua
property tables. All milestones need bounded game examples, behavior/resource
checks and graphics checks on their claimed platforms before promotion.


## Shared live workflow

Coordinate shader/resource reload and editor parameter views with the
[malvado-gui live workflow](live-editing.md). Asset and shader reload do not replace
process code or suspended execution state. Runtime parameter editing precedes
code reload and does not require declarative scenes.
