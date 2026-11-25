# Rust window ownership

Enable the `graphics` feature on `malvado`. Select an installed SDK with the
native graphics component and its raylib archive. Cargo validates signatures,
record layouts and dependencies against that SDK; it does not build native
sources. Default core-only users do not need a graphics backend.

## Managed game

`Game::windowed(width, height, title)` opens a window on the application main
thread. `run()` consumes the game and manages input, native ticks, rendering and
shutdown. It clears to black and draws process graphics, commands, and retained
text. Use `game.audio()` before `run()` to open a shared audio service on demand.
The runner services that device once per render iteration. Exit occurs on
window close, Escape, `p.quit()`, or when no processes remain. All remaining
processes are cancelled before the window lease is released.

Use `p.key_down(Key::Left)`, `p.key_pressed(Key::Space)`, and the corresponding
release, mouse, controller and text methods from any process with a managed
window. Held and edge queries are non-consuming. Text input consumes one
character per successful read. Without a managed window, input returns neutral
values and `p.quit()` reports a platform error. A process token does not keep
the managed window alive. See [runner timing](runner-timing.md#rust-host-integration).

## Manual host

Use `Window` directly when embedding Malvado or controlling the render pass.

```rust
use malvado::{prelude::*, window::Window};
use std::time::Instant;

async fn hello(p: ProcessContext) -> Result<(), Error> {
    loop {
        p.draw_text("Hello, Malvado!", (20, 20), 24, WHITE)?;
        p.frame().await;
    }
}

let mut game = Game::new()?;
let window = Window::open(640, 480, "Manual host")?;
game.spawn(hello)?;
let mut previous = Instant::now();
for _ in 0..3 {
    if window.should_close() { break; }
    let now = Instant::now();
    for _ in 0..game.advance_time((now - previous).as_secs_f64())? {
        window.input_tick();
        game.tick()?;
    }
    previous = now;
    window.frame(BLACK)?.process_graphics(&game)?;
}
drop(game);
# Ok::<(), Error>(())
```

Open the window on the application main thread. The safe wrapper rejects other
threads, invalid dimensions, and embedded NUL in titles. Only one Rust window
can be owned at a time; another open returns `Busy`. Native open failure returns
`Platform`. `Window` cannot move between threads. Do not use another C or Lua
host to close or reopen its native window while it is owned.

`size` reads the logical dimensions, `set_title` updates a copied title, and
`should_close` reads the backend close request. Escape does not close the window
automatically. The game decides when to exit.

Use `window.set_size(640, 360)?` on a desktop GLFW window between frames.
`set_size` returns `InvalidArgument` for a nonpositive dimension, `Busy`
while a frame guard is active, or `Platform` if the backend rejects the
request. Read the current logical dimensions with `size`. The window system
can constrain the requested size or apply the change after processing events.
See the [runtime resize example](../../examples/features/window-resize/README.md).

`frame` begins drawing with a background color. Its borrowed guard ends drawing
and captures platform events on drop, including during unwind. Nested frames
return `Busy`. A forgotten guard prevents another frame; window destruction
finishes it before closing. Dropping the window closes the native service and
resets its input. A new window can then be opened.

The caller controls rendering and simulation timing. This service does not
create another scheduler, advance `Game`, or derive process graphics.
Owned image and immediate drawing services are described in the
[drawing guide](rust-drawing.md). Owned fonts use the same retained-frame model; see the [font guide](rust-font.md).
Audio is covered in the [audio guide](rust-audio.md); process collection uses
the [shared native rendering pass](runner-rendering.md).

`mise run test-rust-graphics` requires a display. It verifies actual open, title,
size, resizing between frames, frame pairing, nested rejection, forgotten guard cleanup, duplicate open,
reopening, main-thread checks, and compile-time frame/thread lifetimes. Both
profiles run from relocated graphics SDKs, including an offline external
consumer. Linux CI runs under Xvfb. Local macOS also runs these checks; no macOS
CI display capability is claimed.

## Keyboard and the native process tick

```rust
use malvado::{Game, window::{Window, Key, Color}};
let mut game = Game::new()?;
let window = Window::open(640, 480, "Game")?;
for _ in 0..3 {
    window.input_tick(); // Once for the shared logical tick, before all readers.
    if window.key_pressed(Key::Escape) { break; }
    game.tick()?;
    drop(window.frame(Color { r: 20, g: 30, b: 40, a: 255 })?);
}
# Ok::<(), malvado::Error>(())
```

`Key` covers the same digits, letters and named controls as C and Lua. The
installed-header probe checks each value. `key_down` reads held state;
`key_pressed` and `key_released` read non-consuming logical-tick snapshots.
They do not add text input, autorepeat or per-process key history.

`input_tick` commits pending key, mouse, and controller edges. Call it once for the shared
window tick, including catch-up ticks, before `Game::tick` or other readers.
Do not call it separately in each process or context. Frames without ticks
retain pending edges. Catch-up ticks clear them without replay. Both edges
can be true when both transitions were observed between ticks. A slower,
sleeping or frozen process can miss a transition in a tick it does not run.
See the [shared input timing contract](keyboard.md).

An `Rc<Window>` lets owned process futures query this one native service without
another input cache. The host remains responsible for commit timing. Dropping
all window owners resets input; reopening starts neutral. The default headless
Game still has no live-window service. Mouse, controller and Unicode text queries are described in the
[input guide](rust-input.md). See the [language matrix](language-parity.md)
for the other delivered platform services.
