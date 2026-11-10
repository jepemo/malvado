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
