# Fade transitions

A fade is a color overlay that starts at transparent black. Enqueue a target
RGBA color with channels in 0..255 and a duration. The native limit is
1,000,000 simulation ticks. A zero duration changes color when the transition
starts.

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
mv.fade_off(500) -- Fade to opaque black over 500 ms.
mv.fade_on(500)  -- Queue a return to transparent black.
```

Use `mv.fade_to {color = mv.colors.BLUE, duration_ms = 500}` to fade to any
[Lua color](drawing.md#lua-colors). All three request functions default to
250 ms when the duration is omitted or `nil`. They accept finite non-negative
numbers, including fractional milliseconds. Positive durations round up to a
whole simulation tick. At 60 Hz, 250, 500, and 1,000 ms take 15, 30, and 60
ticks. The upper limit is 1,000,000 ticks (about 16,666,666.67 ms at 60 Hz).
`fade_to` takes exactly one table and rejects unknown fields. `fade_off` and
`fade_on` take at most one argument. Durations must be numbers, so a string
such as `"500"` raises a Lua error. Invalid arguments and full queues raise
errors without changing the queue or overlay. Successful requests return no
values.

`mv.fade_active()` reports whether a transition or queued request remains.
`mv.fade_color()` returns the current overlay as one RGBA table. Both take no
arguments. The modern global functions behave the same as the module functions.

The fade queue belongs to the running game. It advances after each simulation
tick, including in headless mode, and keeps going when the requesting process
sleeps, freezes, or ends. Durations measure logical simulation time. The
requesting process's FPS, the render rate, and wall time do not determine when
a fade finishes. The runner draws the overlay after process content and
persistent text. A new runner starts with an empty, transparent queue.

### Migrate modern Lua fades from ticks

Milliseconds are a breaking change in the modern Lua API. At 60 Hz, change
`fade_off(30)` to `fade_off(500)` to keep the same duration. Apply the same
conversion to `fade_on`. For `fade_to`, replace `ticks = 30` with
`duration_ms = 500`. The modern function rejects the old key.

### Use tick durations in the legacy profile

```lua
require("malvado.compat.legacy").import_globals()
fade_off(30) -- Fade to opaque black over 30 ticks.
fade_on()    -- Queue a 16-tick return to transparent black.
```

The [legacy import](lua-processes.md#legacy-shim) provides tick-based globals
with a 16-tick default. They use the same queue as the modern module functions.

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

```rust
use malvado::{Error, fade::Fade, window::Color};
let mut fade = Fade::new()?;
fade.enqueue(Color { r: 0, g: 0, b: 0, a: 255 }, 30)?;
fade.step(); // Once per simulation tick, including ticks without a render.
let overlay = fade.color();
# assert!(overlay.a > 0 && overlay.a < 255);
# Ok::<(), Error>(())
```

`enqueue` returns `InvalidArgument` for a duration above the limit, or
`QueueFull` when all pending slots are occupied. `active` includes pending
requests. Rendering does not advance the queue.

## Draw the overlay

In C, `mv_window_draw_fade(color)` draws across the whole logical window.
In Rust, `frame.fade(&fade)` draws the queue's current color. Neither operation
advances a transition. Call it after all scene drawing and after clip/scroll
scopes have ended. Later drawings can cover the overlay. Rust keeps this order
explicit; the Lua runner applies it after persistent text.

The native call returns 0 without drawing if no frame is active, if a clip or
scroll scope is active, or with the headless backend. Rust scopes reject the
call with `InvalidArgument` and preserve their previous drawing state. A fully
transparent overlay succeeds without changing pixels. Each frame starts fresh;
there is no persistent overlay attached to the window.

```rust
# use malvado::{Error, fade::Fade, window::{Color, Window}};
# let window = Window::open(160, 120, "Fade")?;
# let fade = Fade::new()?;
let frame = window.frame(Color { r: 255, g: 255, b: 255, a: 255 })?;
frame.text("Scene", (10, 10), 20, Color { r: 0, g: 0, b: 0, a: 255 })?;
frame.fade(&fade)?; // Last drawing before the frame ends.
drop(frame);
# Ok::<(), Error>(())
```

## Verification

`fade_queue` checks native invalid/NULL calls, duration limits, full-queue
atomicity, repeated ring wraparound, zero-duration draining and decreasing
channels. `alpha7_fade_parity` compares a fixed C/Lua trace against the same
fixture read by Rust. `lua_fade` and `visual_fade` check millisecond conversion
and overlay behavior. `test-rust-graphics` covers Debug/Release, copied values,
installed signatures/layouts, and a relocated consumer. The graphical sanitizer
suite checks the native paths as well. The main-thread `fade_visual` fixture
checks rendered color and alpha composition, transparent no-op, whole-window
coverage after scope restoration, rejected scoped calls without state changes,
frame reset and repeated renders without simulation steps.
