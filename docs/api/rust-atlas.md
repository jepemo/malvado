# Rust atlas metadata

Enable the `assets` Cargo feature on `malvado`. The selected installed SDK must
include the native `assets` component. A core-only SDK still works with default
features; requesting assets from it fails during SDK discovery. Cargo does not
build or download the C library.

```rust
use malvado::atlas::Atlas;
let json = std::fs::read_to_string("characters.json")?;
let atlas = Atlas::parse(&json, 256, 256)?;
if let Some(sprite) = atlas.sprite_named("hero")? {
    println!("{}: {:?}", sprite.name, sprite.region);
}
# Ok::<(), Box<dyn std::error::Error>>(())
```

`Atlas::parse` accepts Malvado schema 1 or 2. Supply the actual image dimensions;
the native parser validates every region against them. It copies the JSON data.
Dropping the input string after parsing is safe. This service loads metadata
only. `image()` returns the declared path; it does not resolve or open a file.

`len`, `image`, `sprite(id)` and `sprite_named(name)` inspect immutable metadata.
Sprite geometry, original canvas size, trim offset and pivot are copied values.
Sprite names and the image path borrow the atlas. Rust prevents these views
from outliving it. Missing sprites return `None`; names are case-sensitive.
Names with embedded NUL return `Error::InvalidArgument`. Native parse failures
return `Error::Asset` with the parser's static diagnostic; this also preserves
its combined invalid-data/allocation failure contract.

The atlas owns one native allocation and frees it on drop. Access and cleanup
stay on the creating thread. `Resource<Atlas>` can retain it after context
shutdown because metadata has no window or device dependency. It does not
retain image data, select graphs, or assign process graphics.

The optional sys bindings check every declared atlas function signature and
all record sizes, alignments and field offsets against the installed headers.
`mise run test-rust-assets` checks Debug/Release, Clippy, borrowed lifetimes,
invalid data and dimensions, lookup behavior, input copying, resource retention,
an offline consumer from a relocated installed SDK, missing-assets diagnostics,
and deliberate header-layout mismatch rejection. Shared C/Rust JSON fixtures
are read from `tests/fixtures`; the C atlas tests also exercise them.

## Aseprite and animation clips

`Atlas::parse_aseprite` imports an unrotated Aseprite JSON array export. The
native parser validates trim offsets, original canvas sizes, animated pivot
slices, durations and supported metadata. Unsupported exports fail explicitly.
Sprites keep their native generated IDs and exact names. `pivot_color()` returns
optional imported RGBA channels as `[u8; 4]`.

`animation_named(name)` returns an optional borrowed `Clip`. Each clip exposes
`len`, `frame(index)`, `sample(elapsed_ms, looping)`, optional `color()` and
optional `repeats()`. Native expansion preserves forward, reverse, ping-pong and
reverse ping-pong order. An invalid index returns `None`. Looping samples wrap;
non-looping samples hold the final frame. An explicit repeat count of zero
stays `Some(0)`; unspecified counts return `None`. Repeats are metadata and do
not change sampling behavior. Names with embedded NUL return `InvalidArgument`.

`clip.to_animation(looping)` copies the expanded sequence into the existing
owned native `Animation`, converting milliseconds to microseconds exactly.
The result survives atlas destruction. Its explicit fixed-tick playback uses
the existing C accumulator. It does not apply repeat metadata, retain graphical
assets, assign graphs, or advance processes automatically. For retained visual playback use the process sprite/animation
APIs below. See the [camera and animation guide](rust-camera-animation.md).

Tests use the same schema 2, Aseprite pivot and ping-pong fixtures as the C suite.
They check frame boundaries, end/loop behavior, directions, colors, repeat
metadata, trim/pivot values, unsupported input, invalid names/indexes, borrowed
clip lifetimes, and owned playback after atlas destruction. The relocated
external consumer imports a clip and samples its independent native animation.

`mise run test-rust-assets-sanitize` runs Rust ownership and atlas tests against
ASan/UBSan-instrumented native core, asset parser and JSON library. Rust code
itself is not sanitizer-instrumented. Both host CI jobs run this check.

Local verification passed: 57 Rust tests and 14 doctests in each profile with
assets enabled, 58 native C tests in each profile, Clippy, format, and the
relocated Debug/Release consumers. Both sanitizer modes pass. The default
core-only SDK checks also pass without enabling assets.

Safe Rust JSON asset constructors serialize access to the native parser
error state. Independent thread owners can import assets concurrently; each
result remains local to its creating thread. Do not overlap imports with raw
C/Lua JSON parser calls on other threads.

To open atlas, Aseprite, FPG or manifest files through one metadata entry point,
use [Library](asset-library-metadata.md#safe-rust-library-loading). It preserves
the Source for referenced-image paths and returns these same owned metadata
wrappers. Actual texture dimensions still require validation before drawing.

`atlas.validate_image(width, height)` checks actual image dimensions through
native metadata, without a window, allocation or JSON reparse. Invalid dimensions
and mismatches return false. Native atlases require region containment; Aseprite
requires the exact declared sheet dimensions. Failed checks leave metadata and
borrowed sprite/animation views unchanged. This also works after Library releases
its source. See [native image validation](asset-library-metadata.md#validate-metadata-against-an-image).

## Atlas textures

With `assets` and `graphics`, `TextureAtlas::new(atlas, image)` pairs owned
metadata with a shared native image. Construction checks every region against
actual image dimensions. Aseprite sheet dimensions must match exactly.
`TextureAtlas::load(window, library)` loads the referenced sheet relative to the
library source path. It accepts Atlas and Aseprite metadata. Other library
formats return `InvalidArgument`.

The texture retains its window. Metadata and image access remain available
through `metadata()` and `image()`. Files must exist during loading; the library
metadata itself uses its owned snapshot. Failed construction releases its input
owners. No process graph assignment or animation update is implicit.

Select an owned `SpriteImage` with `sprite(id)` or `sprite_named(name)`.
Missing entries return `None`. Names are case-sensitive; embedded NUL returns
`InvalidArgument`. Each sprite shares the texture and copies its region and
native cropped pivot. The atlas can then be dropped.

```rust
# use std::rc::Rc;
# use malvado::{atlas::{Atlas, TextureAtlas}, image::Image, window::{Window, Color}};
# let window = Rc::new(Window::open(160, 120, "Atlas sprite")?);
# let image = Image::from_rgba(window.clone(), 1, 1, &[255, 0, 0, 255])?;
# let atlas = Atlas::parse(r#"{"schema":1,"kind":"graphics-library","image":"unused.png","sprites":[{"id":7,"name":"ship","x":0,"y":0,"width":1,"height":1}]}"#, 1, 1)?;
# let textured = TextureAtlas::new(atlas, image)?;
# let background = Color {r:0,g:0,b:0,a:255};
let sprite = textured.sprite_named("ship")?.ok_or(malvado::Error::NotFound)?;
drop(textured);
let frame = window.frame(background)?;
frame.sprite(&sprite, (80.0, 60.0), 1.0, 0.0)?;
# drop(frame);
# Ok::<(), malvado::Error>(())
```

`Frame::sprite` uses the native pivot, positive scale and clockwise rotation.
It uses the existing image-region validation, clip/scroll scopes and frame
retention. Invalid geometry fails before drawing. `region()` and `pivot()`
return copied geometry. `shape()` creates local collision geometry through the
native sprite constructor. Registering that shape and updating its transform
remain explicit host operations. These sprites do not assign process graphs.

## Unified visual loading

With `assets` and `graphics`, `asset::Visual::open(window, path, format)` loads
PNG images, native atlases, Aseprite sheets, FPG files and converted manifests.
`None` or `Some(Format::Unknown)` uses native content routing. Another format
selects that native importer explicitly. Invalid content returns an error.

The result is an owned enum: `Visual::Image(Image)`,
`Visual::Atlas(TextureAtlas)` or `Visual::Fpg(fpg::Textures)`. Match the variant
to use its image, sprite, animation metadata or collection operations. The
loader does not assign a process graph or start automatic animation.

`Visual::from_source(window, source, format)` consumes an existing snapshot.
It does not reopen the source file. PNG decoding uses the snapshot bytes;
atlas and FPG metadata use the native dispatcher. Referenced atlas sheets and
manifest PNG files must still exist, relative to the original source path.
FPG uses index-zero transparency. Manifest PNG alpha is preserved. Use the
specific constructors to choose another FPG transparency policy.

All image creation and cleanup stays on the window owner thread. Selected
sprites and frame draws retain their image data after the Visual is dropped.
A failed load releases its source, metadata and any partial texture collection.
Use `Library` instead when a headless host needs metadata only.

## Process sprite assignment

With `assets` and `graphics`, call `handle.set_sprite(&sprite)` to select a
sprite from an atlas or FPG collection. For a complete PNG or RGBA image, use
`image.sprite((pivot_x, pivot_y))?`. These operations share image references;
they do not copy pixels. Pivots use finite `f64` cropped-image coordinates,
including fractions. Non-finite pivots or magnitudes above `f32::MAX / 16`
return `InvalidArgument` without changing any process.

```rust
use malvado::{Game, Error, image::Image, window::{Color, Window}};
use std::rc::Rc;
let window = Rc::new(Window::open(320, 240, "Process sprite")?);
let image = Image::load(window.clone(), "ship.png")?;
let game = Game::new()?;
let actor = game.spawn(Default::default(), |p| async move {
    loop { p.frame().await; }
})?;
let sprite = image.sprite((0.0, 0.0))?;
actor.set_sprite(&sprite)?;
drop(sprite);
drop(image);
let frame = window.frame(Color { r: 0, g: 0, b: 0, a: 255 })?;
frame.process_graphics(&game)?;
actor.clear_graphic()?;
drop(frame);
# Ok::<(), Error>(())
```

The native process owns the selection and collider geometry. Rust transfers an
image owner that keeps the window alive through replacement, clear, natural
return, kill and context shutdown. The source sprite and image may be dropped
immediately. The caller can also drop its window reference; the graph retains
the window until cleanup. A failed assignment releases temporary references and keeps the
previous selection. Dead handles return `NotFound`; closed contexts return
`Closed`.

`handle.sprite_geometry()` returns copied `SpriteGeometry { region, pivot }`,
or `None` for primitive or absent graphs. It does not expose the native borrowed
image. Cloned handles observe the same selection. `set_graphic` replaces a
sprite with a primitive; `clear_graphic` clears either kind. `graphic()` returns
`None` while a sprite is selected.

`frame.process_graphics(&engine)` draws selected sprites in native z/creation
order with process position, angle, scale, camera, region and scroll. Sleeping
processes are hidden; frozen processes remain visible. The pass completes its
texture uses before returning, so same-frame graph cleanup is safe. Default
bound colliders use the selected sprite geometry. Explicit collider overrides
keep their existing rules. This pass does not tick or advance animation.

Lua uses the same native image graphs and automatic animation. See the
[shared rendering contract](runner-rendering.md).

## Automatic sprite animations

Enable `assets` and `graphics`. `actor.set_sprite_animation(&frames, looping)?`
copies 1–8192 visual frames into
the native process. Each `atlas::SpriteFrame` owns a `SpriteImage` and a
`duration_us` in `1..=3_600_000_000`. Frames may come from whole images, atlases
or FPG collections. The source frames, images and libraries may be dropped
after assignment. Transferred Rust image owners retain the window until cleanup.

```rust
use malvado::{Game, Error, atlas::SpriteFrame, image::Image, window::{Window, Color}};
use std::rc::Rc;
let window = Rc::new(Window::open(320, 240, "Animated process")?);
let red = Image::from_rgba(window.clone(), 1, 1, &[255, 0, 0, 255])?;
let green = Image::from_rgba(window.clone(), 1, 1, &[0, 255, 0, 255])?;
let frames = [
    SpriteFrame { sprite: red.sprite((0.0, 0.0))?, duration_us: 50_000 },
    SpriteFrame { sprite: green.sprite((0.0, 0.0))?, duration_us: 50_000 },
];
let mut game = Game::new()?;
let actor = game.spawn(Default::default(), |p| async move {
    loop { p.frame().await; }
})?;
actor.set_position(40.0, 40.0)?;
actor.set_scale(8.0)?;
actor.set_sprite_animation(&frames, true)?;
drop(frames);
drop(red);
drop(green);
for _ in 0..3 { game.tick()?; }
assert_eq!(actor.animation_clock()?.unwrap().sample.index, 1);
let frame = window.frame(Color { r: 0, g: 0, b: 0, a: 255 })?;
frame.process_graphics(&game)?;
actor.clear_graphic()?;
drop(frame);
# Ok::<(), Error>(())
```

The existing 60 Hz scheduler clock selects images automatically. Process FPS
only changes async activation cadence. Sleep/freeze pause without catch-up;
a non-looping animation holds its final frame. Native process rendering applies
z order, transforms, camera, region and scroll. Default collision geometry is
the fixed local union of all frames, matching Lua animation behavior. Explicit
local/fixed/disabled colliders keep their existing rules.

`animation_clock()` reads the bound state: `sample.index` is zero-based and its
`graph_id` is `index + 1`, rather than an imported asset ID. `sprite_geometry()`
returns the visible frame descriptor. Separate clock assignment/clear returns
`Busy` while the animation is bound. Replace the graph or call `clear_graphic`
to stop playback. Replacement, natural return, kill and context shutdown release
all frame owners. Same-frame cleanup after the native draw pass is safe.

Invalid frames/durations, degenerate union geometry, dead/closed handles or
allocation failure preserve the old graph and clock. Failure does not transfer
Rust owners. Shared frames can be assigned to multiple contexts independently.
Loaded Lua animated descriptors share this native visual binding. Metadata-only
headless descriptors keep their clock-only behavior.
