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
assets, assign graphs, or advance processes automatically. These parts remain
in A7.6. See the [camera and animation guide](rust-camera-animation.md).

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

```rust,ignore
let sprite = textured.sprite_named("ship")?.ok_or(malvado::Error::NotFound)?;
drop(textured);
let frame = window.frame(background)?;
frame.sprite(&sprite, (80.0, 60.0), 1.0, 0.0)?;
```

`Frame::sprite` uses the native pivot, positive scale and clockwise rotation.
It uses the existing image-region validation, clip/scroll scopes and frame
retention. Invalid geometry fails before drawing. `region()` and `pivot()`
return copied geometry. `shape()` creates local collision geometry through the
native sprite constructor. Registering that shape and updating its transform
remain explicit host operations. These sprites do not assign process graphs.
