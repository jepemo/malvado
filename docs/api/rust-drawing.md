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
native-backed drawing checks with ASan/UBSan. This is immediate drawing only;
process draw collection, graph assignment and persistent text remain in A7.6.
