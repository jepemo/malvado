# Rust window ownership

Enable the `graphics` feature on `malvado`. Select an installed SDK with the
native graphics component and its raylib archive. Cargo validates signatures,
record layouts and dependencies against that SDK; it does not build native
sources. Default core-only users do not need a graphics backend.

```rust
use malvado::window::{Window, Color};
let window = Window::open(640, 480, "Game")?;
for _ in 0..3 {
    if window.should_close() { break; }
    let frame = window.frame(Color { r: 20, g: 30, b: 40, a: 255 })?;
    // Draw through delivered frame operations when available.
    drop(frame); // End drawing and process platform events.
}
# Ok::<(), malvado::Error>(())
```

Open the window on the application main thread. The safe wrapper rejects other
threads, invalid dimensions, and embedded NUL in titles. Only one Rust window
can be owned at a time; another open returns `Busy`. Native open failure returns
`Platform`. `Window` cannot move between threads. Do not use another C or Lua
host to close or reopen its native window while it is owned.

`size` reads the logical dimensions, `set_title` updates a copied title, and
`should_close` reads the backend close request. Escape does not close the window
automatically. The game decides when to exit.

`frame` begins drawing with a background color. Its borrowed guard ends drawing
and captures platform events on drop, including during unwind. Nested frames
return `Busy`. A forgotten guard prevents another frame; window destruction
finishes it before closing. Dropping the window closes the native service and
resets its input. A new window can then be opened.

The caller controls rendering and simulation timing. This service does not
create another scheduler, advance `Engine`, or derive process graphics.
Image/font/audio ownership and process draw collection remain in A7.6.

`mise run test-rust-graphics` requires a display. It verifies actual open, title,
size, frame pairing, nested rejection, forgotten guard cleanup, duplicate open,
reopening, main-thread checks, and compile-time frame/thread lifetimes. Both
profiles run from relocated graphics SDKs, including an offline external
consumer. Linux CI runs under Xvfb. Local macOS also runs these checks; no macOS
CI display capability is claimed.
