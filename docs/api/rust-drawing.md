# Rust immediate drawing and images

Enable the `graphics` feature and open a [Window](rust-window.md). Its `frame`
guard keeps drawing active. Calls return `InvalidArgument` when native geometry
or drawing scope validation fails; rejected calls preserve current state.

```rust
use malvado::{drawing::Rectangle, window::{Window, Color}};
let window = Window::open(320, 240, "Shapes")?;
let red = Color { r: 255, g: 0, b: 0, a: 255 };
let frame = window.frame(Color { r: 0, g: 0, b: 0, a: 255 })?;
frame.with_clip(Rectangle { x: 10, y: 10, width: 100, height: 100 }, |frame| {
    frame.rectangle(Rectangle { x: 20, y: 20, width: 40, height: 40 }, true, red)
})?;
drop(frame);
# Ok::<(), malvado::Error>(())
```

`rectangle` and `circle` take integer geometry and a filled flag. Their native
checks reject empty dimensions and integer boundary overflow.
`transformed_rectangle` takes floating-point position, dimensions, local pivot
and clockwise angle. `filled_circle` takes floating-point center and radius.
Geometry must be finite, with positive dimensions or radius. All math and
rasterization stay in the C backend.

`with_clip` accepts an in-window rectangle. `with_scroll` translates by the
negative offset. Start clipping before scrolling. Nested clips, nested scrolls,
and clip changes inside a scroll fail without changing the active scope.
The closure returns `Result`; successful values and errors pass through. Private
guards restore state on return, error or unwind. No user can retain a scope guard
past its frame. Ending the frame also restores native drawing state.

`frame.text(text, position, size, color)` uses the built-in font. Size must be
positive; embedded NUL is invalid. `window.text_width` uses the same native
measurement, including its minimum built-in font size. A zero width means empty
text or a native unrepresentable result. Text does not create a persistent object.

`test-rust-graphics` verifies rendered pixel colors for shapes, transforms,
clipping, scrolling and scope restoration, as well as malformed geometry,
invalid text, nesting, errors and unwind. Debug/Release and a relocated SDK
consumer exercise the delivered API. `test-rust-graphics-sanitize` runs the same
native-backed drawing checks with ASan/UBSan. Process command collection is described below. Persistent text remains in A7.6.

## Owned images

```rust
use std::rc::Rc;
use malvado::{image::Image, window::{Window, Color}};
let window = Rc::new(Window::open(320, 240, "Image")?);
let image = Image::from_rgba(window.clone(), 1, 1, &[255, 0, 0, 255])?;
let frame = window.frame(Color { r: 0, g: 0, b: 0, a: 255 })?;
frame.image(&image, (10, 10))?;
drop(image); // The frame still retains this queued image.
drop(frame);
# Ok::<(), malvado::Error>(())
```

`Image::load(window, path)` loads PNG data through the native weak path cache.
Each successful load owns one native reference. Equivalent resolved paths share
the live texture; dropping the final native reference evicts it. Changing a
file does not reload a live cached image. Missing or unsupported files return
`Platform`; embedded NUL returns `InvalidArgument`. Paths use native Unix bytes.

`from_rgba` accepts dimensions 1..8192 and at least width * height * 4 bytes.
Invalid dimensions or short data return `InvalidArgument`. The backend copies
the packed RGBA data during the call. GPU or native allocation failure returns
`Platform`. `size()` reads native dimensions. `Image::clone()` shares the
immutable Rust owner without another native load or pixel copy.

Each image retains its `Rc<Window>`. Final native image destruction runs before
that window owner is released. Images and cleanup stay on the main thread.
They can be held in `Resource<Image>` and remain usable after process or context
shutdown. Another window cannot open while an image still owns the old window.

`frame.image` queues a complete image at its top-left position.
`frame.image_region` uses a source `Rectangle` and an image `Transform` with
pixel position/pivot, positive scale and clockwise rotation. Native validation
rejects out-of-bounds regions, nonfinite values and scaled geometry overflow.
With `assets` enabled, `atlas::Region` converts into the source rectangle.
This copies geometry and does not retain atlas metadata.

A frame retains each queued image until after native drawing completes. Caller
or native resource-owner destruction during a tick cannot unload a queued
texture. A failed region check queues nothing. Forgetting a frame can leak its
retained images and window, like forgetting another owned Rust resource.

Automated checks verify PNG and RGBA pixels, transparency, region transforms,
input-buffer copying, invalid input, independent native load references,
retention across resource/context destruction, clone/drop, and final window
shutdown/reopen. Both native profiles and the relocated offline consumer pass.
Image selection for process graphs and automatic animation remain in A7.6.

Local verification passes: 57 workspace tests, 17 doctests and four bounded
main-thread graphics executables in Debug and Release, shared C/Lua/Rust input
traces, Clippy, format and relocated offline consumers. The image-owner test
also kills a process after queueing its owned texture and checks pixels before
frame completion. Native graphical ASan/UBSan and the core-only SDK regressions
pass. Linux host CI includes the GLX-scoped leak-filter negative fixture.


## Process drawing commands

Use handle methods to keep drawing between process activations:

```rust
use malvado::{Engine, Error, graphic::Color};
let mut engine = Engine::new()?;
let actor = engine.spawn(Default::default(), |process| async move {
    loop {
        process.current().draw_box((10, 20), (30, 40), true,
            Color { r: 255, g: 80, b: 40, a: 255 })?;
        process.frame().await;
    }
})?;
engine.tick()?;
assert_eq!(actor.draw_count()?, 1);
# Ok::<(), Error>(())
```

`draw_box`, `draw_circle`, and `draw_text` use native ordered storage. They work
without graphics enabled. Geometry and text are copied; embedded NUL text is
rejected. Native integer bounds and failure behavior apply. `draw_count` reports
all native commands, including those appended through C. `clear_draw_commands`
releases the list immediately and is idempotent.

The async adapter clears the old list after cadence and readiness checks, just
before a real future poll. Pending futures without a wake keep their commands.
Sleep and freeze also keep the list. Manual commands queued before the first
poll are cleared at that activation. Process return, kill, and engine shutdown
release the list through native cleanup.

With graphics enabled, `draw_image_region(&image, region, transform)` copies a
region and transform. Native storage retains the image. A transferred Rust
image owner also keeps its window alive. Dropping caller values is safe; clear
and cleanup release the native image before its window owner. Invalid calls
retain neither owner nor command.

Call `frame.process_graphics(&engine)` to draw graphs and commands in native
z/creation order with camera, region and scroll projection. Commands follow the
graph and keep append order. They use their own world coordinates and do not
inherit the graph's position, angle, scale, or collider. This pass does not tick
the engine. Immediate frame drawing remains available for host-owned content.
Persistent text uses [retained text handles and the shared text pass](text.md#rust-retained-text-drawing).
Lua uses the same native [graph/command pass](runner-rendering.md#lua-shared-graph-pass).
