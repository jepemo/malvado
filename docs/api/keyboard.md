# Keyboard state and transitions

Use three operations:

| Lua | C | Meaning |
| --- | --- | --- |
| `key(name)` | `mv_key_down(key)` | The key is currently held. Existing behavior is unchanged. |
| `key_pressed(name)` | `mv_key_pressed(key)` | A press was observed for this logical tick. |
| `key_released(name)` | `mv_key_released(key)` | A release was observed for this logical tick. |

Lua also exposes these operations through the existing `mv` module and input
namespace. `key_down` remains the established alias of `key`. Names and `KEY_*`
constants follow the existing case-sensitive validation. Invalid names raise an
error; headless input is false. C uses `MvKey`; unsupported keys or no open window
return zero. All platform calls stay on the window owner thread.

```lua
if key("right") then
  -- Continuous movement while held.
  self.x = self.x + 3
end
if key_pressed("space") then
  -- Perform this action once, without a previous-key table.
  play_sound(sound)
end
```

Transitions are shared snapshots, not consumable events. Every process and every
read in one tick sees the same result. They stay true for that tick only. A
slow, sleeping or frozen process can miss a transition if it does not execute
in that tick. Use held state for continuous input, or an active process and
messages when a game needs queued actions. No per-process history is maintained.

## Render frames and logical ticks

The backend captures observed press/release transitions after `mv_window_end`
processes platform events. Frames without simulation ticks retain pending
transitions. The next logical tick commits them; additional catch-up ticks clear
them without replaying a press. Multiple observations coalesce into booleans,
not counts. Both pressed and released can be true when both were observed
between ticks; held state can then be false. Hardware transitions not observed
by the backend cannot be reconstructed. These functions are not OS autorepeat
or Unicode text input; keep using the text queue for typing.

The Lua runner calls `mv_input_tick()` once before each windowed logical tick,
including debug steps. Headless runs return neutral input without advancing a
window snapshot. Direct C hosts call it once for their
simulation tick, before reading input or running processes:

```c
mv_input_tick();
MvResult result = mv_process_tick(ctx);
```

Call it for the shared window tick, not for each process or context using that
snapshot. The core scheduler stays independent of platform input. Rendering
alone does not advance the committed snapshot. Closing or reopening the window
resets pending and committed edges. Pause can accumulate transitions until the
next step or resumed tick; these are still coalesced, not a replay queue.

The audio example now uses `key_pressed` for its effect and music actions.
`key_up` is unnecessary because `not key(name)` already expresses it. Repeat,
double-click, toggles, action maps and mouse/controller edge helpers are not
added by this change. Extend other devices only with a concrete use case and
matching timing contracts.

## Verification and Rust status

Deterministic C tests cover frames without ticks, repeated reads, multiple
process readers, catch-up ticks, press/release coalescing, invalid keys and window
reset. A backend integration test checks two Lua readers, lower process cadence,
and reopening the runner. The existing Linux keyboard injection test also checks
actual backend press/release transitions. Headless Lua tests cover validation,
module/global/namespace aliases, and neutral state.

Rust provides `Window::key_down`, `Window::key_pressed` and
`Window::key_released` with the typed `Key` enum under the `graphics` feature.
The host calls `Window::input_tick()` once before its shared logical tick,
then runs `Engine::tick()`. Processes can share an `Rc<Window>` for queries;
reads use the same native snapshot and never consume an edge. The window and
all its readers stay on the application main thread. Rendering through a frame
guard captures backend events on drop, but does not commit a logical tick.

The default headless Rust Engine has no window or live input service; enabling
this feature does not create one implicitly. Invalid keys cannot be constructed
through the safe enum. The C unavailable/invalid-key rules remain unchanged.
See the [Rust window guide](rust-window.md) for ownership and host-loop usage.

`test-rust-graphics` compares one shared five-tick golden trace against native
C, Lua and Rust readers in both profiles, including multiple readers and a
30 FPS process. The Rust fixture also checks zero-tick frames, catch-up ticks,
non-consuming queries and reset after closing/reopening. Linux runs the same
real keyboard injection script against the C and Rust backends, including held
state, one press and one release for each tested key. Lua headless validation
and the audio example action checks remain in the existing native suites.
