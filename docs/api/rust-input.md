# Rust mouse, gamepad and text input

Enable `graphics` and use the same live [Window](rust-window.md) as the native
keyboard service. No extra input loop, cache or process scheduler is created.
The default headless Engine has no live device service.

```rust
use malvado::window::{Window, MouseButton};
let window = Window::open(640, 480, "Input")?;
let (x, y) = window.mouse_position();
if window.mouse_down(MouseButton::Left) {
    println!("Pointer held at {x}, {y}");
}
while let Some(character) = window.text_input_next() {
    print!("{character}");
}
# Ok::<(), malvado::Error>(())
```

`mouse_position` returns logical window pixels. It does not apply camera or
scroll transforms. `mouse_down` takes typed Left, Middle or Right buttons.
These read native held state and do not consume it.

`pad_connected(slot)`, `pad_button_down(slot, PadButton)` and
`pad_axis(slot, PadAxis)` use the native virtual controls. Slots are 0..3;
other `u8` values return false/zero. Missing controllers or axes also return
neutral values. Stick axes stay in -1..1 and triggers in 0..1, with native
normalization. Rust adds no dead zone, mapping database or device registry.
Every enum value and its native size/alignment is checked against the SDK.

`text_input_next` removes one Unicode scalar and returns a Rust `char`, or
`None` when the shared queue is empty. Unlike held-state queries or keyboard
transition snapshots, text reads consume input across all processes/readers.
Capture runs when a frame guard ends and processes backend events. No input
tick commit is needed to read text. The queue retains up to 256 scalars and
drops new arrivals when full. Invalid scalars and NUL are not queued. Closing
or reopening the window clears it. Keyboard queries are separate from typing.

All calls stay on the window owner thread. Scope and native ownership rules
match the keyboard/window service. No new mouse/pad edge, repeat or action-map
APIs are added.

`test-rust-graphics` checks disconnected/invalid slots, enum ABI, neutral text,
Unicode including non-BMP scalars, rejected scalars, shared consumption, queue
capacity and reopen/reset. A private backend sampling seam drives deterministic
Unicode tests; production event capture uses that same append operation. Linux
also runs real mouse/button and text injection against Rust with the existing
C/Lua scripts in Debug/Release. The relocated consumer and native graphical
sanitizers cover these wrappers. Connected-controller button/axis state is not
claimed as tested on runners without that device or a controller emulator.
