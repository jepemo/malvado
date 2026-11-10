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
