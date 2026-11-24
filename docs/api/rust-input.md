# Rust mouse, gamepad and text input

Enable `graphics` and use the same live [Window](rust-window.md) as the native
keyboard service. No extra input loop, cache or process scheduler is created.
The default headless Game has no live device service.

```rust
use malvado::window::{Window, MouseButton};
let window = Window::open(640, 480, "Input")?;
let (x, y) = window.mouse_position();
window.input_tick();
if window.mouse_pressed(MouseButton::Left) {
    println!("Pointer pressed at {x}, {y}");
}
while let Some(character) = window.text_input_next() {
    print!("{character}");
}
# Ok::<(), malvado::Error>(())
```

`mouse_position` returns logical window pixels. It does not apply camera or
scroll transforms. `mouse_down` takes typed Left, Middle or Right buttons.
Use `mouse_pressed` and `mouse_released` for non-consuming transitions in the
current logical tick. Call `window.input_tick()` once before each logical
tick. Any readers of that tick use the same snapshot.

`pad_connected(slot)`, `pad_button_down(slot, PadButton)` and
`pad_axis(slot, PadAxis)` use the native virtual controls. Slots are 0..3;
other `u8` values return false/zero. Missing controllers or axes also return
neutral values. Stick axes stay in -1..1 and triggers in 0..1, with native
normalization. Rust adds no dead zone, mapping database or device registry.
Use `pad_button_pressed` and `pad_button_released` for per-tick transitions.
When a device connects, the first observed held state establishes a baseline.
Disconnecting clears edges and gives neutral input for that slot. Every enum
value and its native size and alignment are checked against the SDK.

`text_input_next` removes one Unicode scalar and returns a Rust `char`, or
`None` when the shared queue is empty. Text reads consume input across all
readers. Key, mouse, and controller held-state and transition reads are
non-consuming.
Capture runs when a frame guard ends and processes backend events. No input
tick commit is needed to read text. The queue retains up to 256 scalars and
drops new arrivals when full. Invalid scalars and NUL are not queued. Closing
or reopening the window clears it. Keyboard queries are separate from typing.

All calls stay on the window owner thread. Scope and native ownership rules
match the keyboard/window service.

On Android, physical key-down events fill the same native text queue through
`KeyCharacterMap`. Modifier state and key repeats apply. Control characters,
DEL and dead-key markers are discarded. Exported Android games can request
the [on-screen keyboard](keyboard.md#android-on-screen-keyboard) with
`window.set_text_input_keyboard(true)` and hide it with the same method.
