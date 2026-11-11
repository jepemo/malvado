# Fade transitions

A fade is a color overlay. Start at transparent black. Enqueue a target RGBA
color and a duration in simulation ticks. Color channels are 0..255. Duration is
0..1,000,000. A zero duration changes color when the transition starts.

C, Lua and Rust use one native FIFO implementation. It holds one active timed
transition and up to 16 pending requests. An idle queue starts immediately.
A busy queue appends. A full queue rejects the new request and preserves all
previous work. Invalid input also preserves previous work.

Advance once per simulation tick, independently of rendering. Each step first
advances the current transition, then starts queued work if it completed.
Queued zero-duration transitions drain on that same step. A newly started
timed transition first advances on the next tick. Interpolation uses integer
channel differences, truncated toward zero. For example, a two-tick alpha fade
from 0 to 255 samples 0, 127, 255. From 255 to 0 it samples 255, 128, 0.

## Lua

```lua
mv.fade_off(30) -- Fade to opaque black.
mv.fade_on(30)  -- Queue a return to transparent black.
```

`mv.fade_to(r, g, b, a, ticks)` chooses a color. `fade_off` and `fade_on`
default to 16 ticks. `mv.fade_active()` includes queued work.
`mv.fade_color()` returns the current four channels. Invalid values or a full
queue raise a Lua error. The runner advances after each process tick and draws
the overlay after process content and persistent text. A new runner starts
with an empty, transparent queue. Fades also advance in headless mode.

## C

Include `malvado/platform.h`. The functions work without a window or Lua.
Link the selected platform library, including its headless implementation.

```c
MvFadeQueue fade = {0};
int result = mv_fade_queue_push(&fade, (MvColor){0, 0, 0, 255}, 30);
/* result is 1 on success, 0 for invalid input, or -1 for a full queue. */
mv_fade_queue_step(&fade); /* Once per simulation tick. */
MvColor overlay = fade.fade.current;
```

Initialize the queue to zero and use its operations to change it. Read the
current color but do not modify internal fields. `mv_fade_queue_active` reports
active or queued work. `step` returns 1 when it changes queue state and 0 when
idle. NULL returns 0 for all three operations. The existing `MvFade` functions
remain available for a single transition without a queue. C callers choose
when and where to draw their overlay.

## Rust

Enable `graphics` to link the installed platform services. `Fade` itself needs
no window, audio device, process, or scheduler. It is a copied value with
independent state. Rust does not implement interpolation or queue policy.

```rust,no_run
use malvado::{Error, fade::Fade, window::Color};
let mut fade = Fade::new()?;
fade.enqueue(Color { r: 0, g: 0, b: 0, a: 255 }, 30)?;
fade.step(); // Once per simulation tick, including ticks without a render.
let overlay = fade.color();
# Ok::<(), Error>(())
```

`enqueue` returns `InvalidArgument` for a duration above the limit, or
`QueueFull` when all pending slots are occupied. `active` includes pending
requests. Rendering does not advance the queue.

## Verification

`fade_queue` checks native invalid/NULL calls, duration limits, full-queue
atomicity, repeated ring wraparound, zero-duration draining and decreasing
channels. `alpha7_fade_parity` compares a fixed C/Lua trace against the same
fixture read by Rust. `lua_fade` and `visual_fade` preserve existing Lua timing
and overlay behavior. `test-rust-graphics` covers Debug/Release, copied values,
installed signatures/layouts, and a relocated consumer. The graphical sanitizer
suite checks the native paths as well.
