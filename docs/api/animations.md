# Automatic process animation

Assign an animation to `self.graph`. Each process plays it independently on the
existing simulation clock. A static `graph()` stays on the selected graphic.

```lua
require("malvado").import_globals()
local hero
local Player = process(function(self)
  self.file = hero
  while true do
    self.graph = animation("walk")
    frame()
  end
end)
start(function()
  hero = load_asset("hero.json")
  Player({x = 100, y = 100})
end)
```

Use `animation(name, options?)`, `animation(library, name, options?)`, or
`library:animation(name, options?)` for native atlas/Aseprite sequences.
The explicit namespace is `mv.graphics.animation`; `mv.animation` is an alias.
Implicit forms require the actual scheduled process and its `self.file`.
Explicit forms work without that context. All selectors require asset support.

For FPGs, converted manifests, or atlases with individual graphic IDs, define a
sequence once and reuse it:

```lua
local walk = animation(library, {7, 42, 7}, {fps = 8, loop = true})
-- Inside a process:
self.graph = walk
```

Custom ID lists contain 1..4096 entries; repeated and nonconsecutive IDs work.
Every ID must exist. `fps` defaults to 8 and must be finite in 1/3600..1000.
Uniform durations are rounded to the nearest microsecond. `loop` defaults to
true and must be boolean. Named sequences preserve their native frame durations
and tag direction; they accept `loop`, not an fps override. Aseprite repeat-count
metadata retains its existing descriptive meaning; it is not a playback limit.
Unknown options, names, IDs, sparse lists and invalid contexts raise errors.
Per-frame custom duration tables are not part of this initial interface.

## Playback and lifetime

Definitions are immutable and shareable. Named lookup reuses cached identity
while the definition is in use, so assigning the same animation each tick does
not restart it or create a new definition. Switching to another definition
starts at its first frame. Removing and then reassigning a definition starts it
again. Create custom definitions once instead of rebuilding a list each tick.

A new selection displays its first frame in that tick. Subsequent running ticks
advance playback, even when process-local fps reduces behavior activations.
Sleeping processes remain hidden and pause playback. Frozen processes keep the
current frame visible and pause playback. Wakeup resumes without catching up
paused time. Non-looping sequences hold their final frame. Rendering does not
advance time. Both Lua game entry forms use the same rules.

Descriptors retain their source library and frame graphics/images. Changing
`self.file` does not reinterpret a selected sequence. A process's graph reference
keeps its definition alive; normal garbage collection releases unused definitions.
The native definition owns its copied timing data. No extra scheduler is used.
Headless libraries can define geometry/timing without images, with the same
metadata-only limits as static graphs.

The default collider is the fixed union of every frame's local rectangle,
including pivots and trim offsets. This prevents geometry from changing during
playback. It can be larger than an individual frame. Use an explicit `self.collider`
for tighter gameplay geometry, or false to disable it. Nil restores the union.
World/process collision participation, ordering and transforms remain unchanged.

The [Aseprite example](../../examples/aseprite/) is a runnable named sequence
with its PNG/JSON included. See the [migration guide](../development/alpha6-api-migration.md)
for static/manual and explicit namespace alternatives.

## Explicit sampling

`definition:sample(elapsed_ms)` returns the reusable static frame descriptor at
a finite time in 0..1e12 milliseconds. It uses the definition's loop rule and
does not change any process. Use `graph(library, id)` for a specific static ID.
The existing `animation_frame(atlas, name, elapsed_ms, loop?)` sampler remains.

`mv.graphics.animation_elapsed(process)` returns playback milliseconds within
the sequence (wrapping when it loops), or nil for a static graph. It is an
advanced inspection/synchronization helper; dead or invalid handles raise errors.
The update happens after scheduled process behavior, so a process observes the
completed previous tick's advance until that tick's update finishes.

## Native C

Include `malvado/animation.h` and link `malvado_core`. Create an immutable
`MvAnimation` from copied `MvAnimationFrame` values (positive IDs, durations in
1..3600000000 microseconds, up to 8192 expanded entries). The larger native limit
covers expanded Aseprite ping-pong tags. Each player owns a separate
`MvAnimationState state = {0}`. Call `mv_animation_advance` once per running
simulation tick, then `mv_animation_sample` for its zero-based index and ID.
Keep the definition alive until all callers finish; destroy accepts NULL.
Creation returns NULL for invalid input or allocation failure. Sample/advance
return 0 on invalid arguments without changing output/state. Use a fixed rate
in 1..1000000; reset state before changing it. Looping state stays bounded.

C games keep resource, draw and collider control explicit. Select a fixed collider
that covers the sequence's local frame bounds to use Lua's default union policy.
Native atlas helpers `mv_atlas_animation_frame_count` and
`mv_atlas_animation_frame_at` expose expanded IDs and millisecond durations as
copied records, without retaining caller buffers. Count is zero for a missing
name; frame lookup returns 0 without changing output on invalid input.

The core definition/player has no Lua, asset-parser or graphics dependency.
The binding handles implicit context, named caching and graph ownership; it uses
the same native playback math as C callers. C games advance state from their
existing scheduler callback, not a new animation scheduler.


The [native C animation example](../../examples/c/animation/) advances player
state from the shared scheduler without Lua or graphics. Its definition is
caller-owned and released after the context stops its users. C-only and
installed SDK checks run the example.
