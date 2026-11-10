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
Owned image and immediate drawing services are described in the
[drawing guide](rust-drawing.md). Owned fonts use the same retained-frame model; see the [font guide](rust-font.md).
Audio and process draw collection remain in A7.6.

`mise run test-rust-graphics` requires a display. It verifies actual open, title,
size, frame pairing, nested rejection, forgotten guard cleanup, duplicate open,
reopening, main-thread checks, and compile-time frame/thread lifetimes. Both
profiles run from relocated graphics SDKs, including an offline external
consumer. Linux CI runs under Xvfb. Local macOS also runs these checks; no macOS
CI display capability is claimed.

## Keyboard and the native process tick

```rust
use malvado::{Engine, window::{Window, Key, Color}};
let mut engine = Engine::new()?;
let window = Window::open(640, 480, "Game")?;
for _ in 0..3 {
    window.input_tick(); // Once for the shared logical tick, before all readers.
    if window.key_pressed(Key::Escape) { break; }
    engine.tick()?;
    drop(window.frame(Color { r: 20, g: 30, b: 40, a: 255 })?);
}
# Ok::<(), malvado::Error>(())
```

`Key` covers the same digits, letters and named controls as C and Lua. The
installed-header probe checks each value. `key_down` reads held state;
`key_pressed` and `key_released` read non-consuming logical-tick snapshots.
They do not add text input, autorepeat or per-process key history.

`input_tick` commits the native pending edges. Call it once for the shared
window tick, including catch-up ticks, before `Engine::tick` or other readers.
Do not call it separately in each process or context. Frames without ticks
retain pending edges. Catch-up ticks clear them without replay. Both edges
can be true when both transitions were observed between ticks. A slower,
sleeping or frozen process can miss a transition in a tick it does not run.
See the [shared keyboard timing contract](keyboard.md).

An `Rc<Window>` lets owned process futures query this one native service without
another input cache. The host remains responsible for commit timing. Dropping
all window owners resets input; reopening starts neutral. The default headless
Engine still has no live-window service. Mouse, controller and Unicode text queries are described in the
[input guide](rust-input.md). Other platform families remain in A7.6.

The SDK consumer also checks that a core-only prefix rejects the graphics
feature and that a changed native color layout fails before graphics calls.
Reconfiguring the same CMake consumer from one SDK prefix to another resolves
raylib from the new prefix rather than a cached path.

`mise run test-rust-graphics-sanitize` checks frame cleanup (including unwind),
window shutdown/reopen, and native keyboard snapshot ownership against an
ASan/UBSan-instrumented backend. The native libraries are instrumented; Rust
itself is not. The macOS runtime uses the existing leak-detection limitation
from the sanitizer task. Linux CI runs the graphical sanitizer check under Xvfb.

Linux LeakSanitizer keeps detection enabled for Malvado allocations. The
verification-only pixel helper pins loaded DRI/Gallium/LLVM modules so
exit-time reports keep their module names and global roots. Games never call
that helper. GLX display-provider modules are not pinned, so close/reopen can
reset their display state. Native window/context and image cleanup still run
normally.
The graphical filter excludes provider-module allocation stacks (`libgallium`,
DRI and LLVM) and the observed GLX extension-query stack. It does not suppress
Malvado draw or allocation frames. Core and asset modes do not use the filter.
A negative fixture verifies that unrelated native allocations still fail.

This limit is based on [GLX query evidence](https://github.com/jepemo/malvado/actions/runs/42829814334/job/129876608971)
and [unloaded drawing-provider stacks](https://github.com/jepemo/malvado/actions/runs/42833417060/job/129889149968).
Changing to softpipe did not remove provider allocations, so tests retain the
default renderer. Provider leak freedom is not claimed by these checks.
