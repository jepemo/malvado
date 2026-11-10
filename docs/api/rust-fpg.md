# Rust FPG import and conversion

Enable the `assets` feature and select an installed SDK with native asset support.
The Rust API uses the same C parser as Lua. It accepts the
[documented uncompressed BennuGD 8-bit subset](../specs/fpg-8.md), not every DIV
FPG variant. Other depths, compressed files and binary writing are unsupported.

```rust
use malvado::fpg::Fpg;
let data = std::fs::read("graphics.fpg")?;
let library = Fpg::parse(&data)?;
if let Some(sprite) = library.sprite(7) {
    println!("{} x {}", sprite.info.width, sprite.info.height);
    println!("{} indexed pixels", sprite.pixels.len());
}
# Ok::<(), Box<dyn std::error::Error>>(())
```

Parsing copies accepted data into a native owned library. Caller bytes can be
dropped immediately. Native parsing or allocation failures return `Error::Asset`
with a static diagnostic. No parser, palette math or format rules are duplicated
in Rust. The native 64 MiB limit and all ID/size/point limits remain authoritative.

`len`, `sprite(id)` and zero-based `at(index)` inspect immutable records in file
order. Missing IDs or indexes return `None`. `SpriteInfo` copies dimensions,
ID, point count and the fixed name/filename byte arrays. These bytes have no
assumed encoding or terminating NUL. Pixel indexes, the 768-byte six-bit RGB
palette and the 576-byte gamma block borrow the owning library.

`point(id, index)` returns copied signed coordinates and a `defined` flag.
An undefined `(-1,-1)` point is distinct from a missing point (`None`). Import
does not apply gamma or infer a transparency policy.

Dropping `Fpg` releases its native data once. Borrows cannot outlive it. Calls
and cleanup stay on the creating thread. `Resource<Fpg>` can retain the library
after its process or context ends; metadata requires no window or device.

The installed-header probe checks native FPG signatures and all public record
sizes, alignments and field offsets. `test-rust-assets` uses the same original
fixture as C/Lua and checks copying, indexed data, raw metadata, control points,
invalid lookups, every truncation boundary, rejected variants, ownership and a
relocated offline SDK consumer in Debug/Release. The asset sanitizer task checks
native allocation/cleanup; Miri does not interpret the C format parser.

## Conversion and manifests

`rgba(id, transparent_zero)` returns owned RGBA bytes. Six-bit palette channels
are shifted left two bits (63 becomes 252). Index zero is transparent only when
requested. Gamma is preserved as metadata and is not applied to colors.

`png(id, transparent_zero)` returns an indexed PNG without creating a file.
It preserves indexes and palette colors, with optional index-zero transparency.
The native encoder uses uncompressed DEFLATE. `json(transparent_zero)` returns
owned UTF-8 manifest JSON without a C terminator; names and filenames remain
lossless byte arrays in that format. Use the same transparency choice for PNG
and JSON. Missing sprite IDs return `NotFound`. Rust buffer allocation failure
returns `OutOfMemory`; invalid native size/copy results return `InvalidArgument`.

```rust
use malvado::fpg::{Fpg, Manifest};
let bytes = std::fs::read("graphics.fpg")?;
let library = Fpg::parse(&bytes)?;
std::fs::write("7.png", library.png(7, true)?)?;
let json = library.json(true)?;
let manifest = Manifest::parse(&json)?;
drop(library); // The manifest owns an independent copy of its metadata.
assert!(manifest.transparent_zero());
# Ok::<(), Box<dyn std::error::Error>>(())
```

`Manifest` supplies `len`, zero-based `at`, `sprite`, `point`, borrowed `palette`
and `gamma`, and `transparent_zero`. Metadata lookup returns copied `SpriteInfo`.
It contains no pixel data and does not load or validate PNG files. Parsing
accepts the existing native version 1 manifest contract, up to 16 MiB, and
reports static native failures through `Asset`. The owner/thread and resource
retention rules match `Fpg`.

With both `assets` and `graphics`, `library.image(window, id, transparent_zero)`
creates an owned `Image` through native RGBA conversion. That image retains its
window, and can be queued with the existing frame API. It survives FPG destruction.
Control points are not applied as automatic pivots. Loading manifests as graph
libraries and automatic process graph assignment remain A7.6 work.

Tests check exact RGBA bytes, indexed PNG structure/transparency, lossless
manifest metadata round trips, malformed manifests, missing IDs, ownership and
borrow/thread compile failures. Graphics checks compare pixels from direct FPG
conversion and an exported PNG decoded by the native loader after the source
library is destroyed. Both native profiles, relocated offline consumers and
native ASan/UBSan cover these operations. The Rust code itself is not sanitizer
instrumented, and the existing documented graphics-driver exclusions still apply.

Safe Rust atlas, Aseprite and FPG-manifest imports serialize native JSON parsing
because cJSON writes a global error record. Independent thread owners can call
these safe constructors; each result still stays on its creating thread.
Raw C/Lua JSON parser calls on other threads must not overlap those imports.
Concurrent cross-language parsing is not exposed as a supported Rust workflow.
