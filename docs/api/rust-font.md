# Rust font ownership and Unicode glyphs

Enable `graphics` and open an `Rc<Window>`. Fonts use the existing native
TTF/OTF loader; they do not add a renderer or process update loop.

```rust
use std::rc::Rc;
use malvado::{font::Font, window::{Window, Color}};
let window = Rc::new(Window::open(320, 240, "Text")?);
let font = Font::load(window.clone(), "font.ttf", 24)?;
let frame = window.frame(Color { r: 0, g: 0, b: 0, a: 255 })?;
frame.font_text(&font, "Café", (10, 10), 24, Color { r: 255, g: 255, b: 255, a: 255 })?;
drop(font); // Queued text retains the font until drawing finishes.
drop(frame);
# Ok::<(), malvado::Error>(())
```

`Font::load` loads the native ASCII and Latin-1 glyph set at base size 1..256.
`load_glyphs(window, path, base_size, glyphs)` accepts 1..1024 distinct Rust
`char` values. NUL and duplicates are invalid. Rust chars cannot contain Unicode
surrogates. The backend copies the glyph list during loading. An unloaded glyph
uses native fallback behavior; glyphs are not loaded automatically during draw.

`text_width` and `frame.font_text` accept drawing size 1..1024 and UTF-8 text.
Embedded NUL is invalid. Width zero means empty text or an unrepresentable
native result. Coordinates use window pixels and the existing frame clip/scroll
state. Invalid sizes or glyph sets return `InvalidArgument`; native load failure
returns `Platform`, including unsupported files, missing files or allocation
failure. Native paths use Unix bytes.

Each successful load owns one native font. `Font::clone` shares that immutable
owner without loading another texture. The font retains its window and stays
on the creating thread. `Resource<Font>` can retain it after a process/context
ends. Frames retain each queued font until after native drawing completes, so
owner termination cannot release a texture that queued text still uses. Native
font destruction precedes final window release. Forgetting a frame can leak its
retained resources, as with other forgotten Rust owners.

Debug/Release and relocated SDK consumers verify loading, measurement, invalid
input, rendered glyphs, Unicode selection versus fallback, process resource
cleanup before draw completion, clone/drop, and final window shutdown/reopen.
The native graphical sanitizer task covers this path with its documented driver
limits. This service does not create persistent text objects or auto-layout.
