# Drawing defaults

The native drawing-state value stores a drawing color and a background color.
It starts with opaque white drawing and opaque black background. It owns no
window, process, or resources. Copies are independent.

In C, include `malvado/platform.h` and link the selected platform library.
Call `mv_draw_state_init(&state)` before use. `mv_draw_state_color` takes four
signed 64-bit RGBA channels; `mv_draw_state_background` takes three RGB channels
and sets alpha to 255. Each channel must be in 0..255. These functions return 1
on success, or 0 for NULL or invalid input. Failure preserves the whole state.
Read the fields but use these operations to change them. Nothing is drawn or
changed in a window by a state update.

Lua keeps one state per runner. `mv.set_color(r, g, b[, a])` uses alpha 255 by
default. `mv.clear_screen(r, g, b)` selects the opaque background for future
renders. Invalid input raises a Lua error without changing either default.
Starting a runner resets both defaults. Immediate process commands capture the
current drawing color when queued; later state updates do not recolor them.
The existing process scheduler and draw order are unchanged.

Rust uses `drawing::DrawState` with the `graphics` feature. It does not require
an open window. `new()` initializes the native defaults and checks the SDK ABI.
`set_color(Color)` and `set_background((r, g, b))` use typed byte channels, so
invalid channel values cannot be passed. `color()` and `background()` return
copies. Use these as explicit arguments in the normal frame API:

```rust,no_run
use malvado::{Error, drawing::{DrawState, Rectangle}, window::{Color, Window}};
let mut state = DrawState::new()?;
state.set_background((10, 20, 30));
state.set_color(Color { r: 255, g: 0, b: 0, a: 255 });
let window = Window::open(160, 120, "Drawing defaults")?;
let frame = window.frame(state.background())?;
frame.rectangle(Rectangle { x: 10, y: 10, width: 20, height: 20 }, true, state.color())?;
drop(frame);
# Ok::<(), Error>(())
```

Changing a Rust value does not mutate a live frame or install global state.
Rendering only uses a selected color when the caller passes it to a frame or
drawing operation. Fades remain a separate [native queue](fades.md).

## Verification

`draw_state` checks native defaults, signed channel boundaries in every position,
NULL, atomic failures and independent copies. The Rust value test checks all
byte-channel values and copied state. The Rust drawing fixture checks pixels,
alpha blending, explicit frame backgrounds and updates without implicit drawing.
The Lua fade and process-graphics fixtures check colors and backgrounds after
failed updates.
Standard/headless and C-only suites preserve runner behavior. Debug/Release,
installed SDK signatures/layouts, the relocated graphics consumer and native
ASan/UBSan cover the Rust boundary and drawing paths.
