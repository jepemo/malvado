# Animation

Make a sprite cycle through several frames. Give each frame its own
duration, then let the process keep playing the sequence between the process’s Lua activations.

## Run the example

Install the [desktop SDK](../../start/installation/) and extract the ZIP
into an empty directory. Keep `main.lua`, `sheet.json` and `sheet.png`
together. Run this command there.

```sh
malvado run main.lua
```

The colored sprite repeatedly changes frames. Press Escape or close the
window to exit. The archive includes the Apache 2.0 license and credits
for the original Malvado PNG and JSON.

<!-- example -->

## Let the graph play

The process sets `self.file` to the atlas and selects its named
`"bounce"` animation. The tag plays frames one, two, three, then two
again. Each frame keeps its duration from the atlas. Their durations are
100, 200, 300 and 200 milliseconds, so a full cycle takes 800 milliseconds.
To define a uniform sequence yourself, pass a library, an array of graphic
IDs and options such as `{fps = 8, loop = true}` to `animation`.

The animation descriptor is reusable. Each process that uses it has its
own playback time. Playback advances on running simulation ticks, even
if the Lua process activates less often. Sleeping and frozen processes
pause their animation. Replacing the selected animation with a different
definition starts the new one at its first frame. The graph retains the
definition, which keeps the library and frames alive.

## Play once

Change the selection to `animation("bounce", {loop = false})`. It plays
through once and then holds the last frame. For an effect that should
disappear, let its process return when `animation_finished()` becomes
true. That happens after the last frame's full duration. A looping sequence
never reports completion.

The default animation collider covers the union of the frames' bounds,
so it can be larger than the frame you currently see. Set an explicit
collider when a game needs a tighter hit area.

See the [Lua animation reference](../../reference/lua/graphics/animation/) and
the [animation guide for C and Lua](../../concepts/animations/).
Experimental Rust has [native animation clocks and
sampling](../../concepts/rust-camera-animation/).
