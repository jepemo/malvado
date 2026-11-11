# Unified library metadata import

Use the native metadata dispatcher to load an atlas, Aseprite export, binary
FPG or converted FPG manifest from an owned asset source. It selects an existing
native importer and returns owned, validated library data. No Lua runtime,
context, graphics backend or window is needed.

```c
#include "malvado/asset.h"

const char *error = NULL;
MvAssetSource *source = mv_asset_source_open("sheet.json", &error);
MvAssetMetadata metadata = {0};
if (source) {
    if (mv_asset_metadata_parse(source, MV_ASSET_UNKNOWN, &metadata, &error)) {
        if (metadata.atlas) {
            size_t sprites = mv_atlas_count(metadata.atlas);
            (void)sprites;
        }
    }
    mv_asset_source_destroy(source);
}
/* Parsed data does not borrow the source. */
mv_asset_metadata_destroy(&metadata);
```

`MV_ASSET_UNKNOWN` selects the content hint from the source. An explicit atlas,
Aseprite, FPG or manifest format selects that importer instead. The existing
importer validates the complete schema or binary structure. A routing hint alone
does not make metadata valid. PNG and unknown/unsupported formats fail. Use the
image loader for PNG; it has no library metadata to import here.

On success, the record has the selected format and exactly one non-NULL field:
`atlas`, `fpg` or `manifest`. Aseprite uses the atlas field with format
`MV_ASSET_ASEPRITE`. FPG data includes owned CPU pixel indexes, palette and point
metadata. The record owns no texture, image, device, context or source reference.
Source bytes and files can be released after successful import.

Initialize records to `{0}`. Do not shallow-copy owning records. Use the matching
existing atlas/FPG APIs with the owned pointers. To move one allocation into a
new owner, copy its pointer and set that field to NULL in the record. The new
owner then destroys or registers it once. `mv_asset_metadata_destroy` releases
all remaining allocations and resets the record. NULL and repeated destruction
of the reset record are allowed. Destroy old data before reusing an output.

Parsing returns 1 on success and sets the optional static error to NULL. It
returns 0 for NULL source/output, invalid format, bad metadata or allocation
failure. Failure preserves the output and source, releases partial importer
allocations and reports a static error. All access and destruction use the
owner thread. Serialize parsing with other native JSON imports.

Atlas geometry is validated against advisory Aseprite `meta.size`, or INT_MAX
when image dimensions are unavailable. Before rendering, a host must validate
against actual image dimensions. The dispatcher does not open referenced files,
load textures or apply FPG transparency to images. Those operations remain in
the visual resource layer. This distinction matches headless Lua loading.

Lua `load_asset` uses this dispatcher for all four library formats. Private GC
userdata guards metadata before later Lua allocations. Ownership moves into the
existing atlas/FPG userdata and clears the guard's pointer. With a window, Lua
still loads referenced images and revalidates atlas bounds against the actual
texture. Explicit format options, FPG transparency, imported animation metadata,
image retention and ordinary parser entry points retain their behavior.

`asset_metadata` checks real files, auto/explicit error dispatch, source
independence, ownership moves, idempotent cleanup, invalid arguments, failure
output preservation and converted manifests. `asset_metadata_allocation` injects
failures across every allocation in all four import paths and checks that no
allocation remains. Existing headless and graphical Lua loader/resource tests
cover the moved paths. Installed Rust probes check both signatures and every
field's size, alignment and offset. Unified texture construction, process asset
graphs and automatic animation remain separate alpha.7 work.

## Safe Rust library loading

With the `assets` feature, use `malvado::asset::{Library, Metadata, Format}`.
Library opens an owned Source and calls the same native metadata dispatcher.

```rust
use malvado::asset::{Library, Metadata};
let path = std::env::temp_dir().join(format!("malvado-library-doc-{}.json", std::process::id()));
std::fs::write(&path, r#"{"schema":1,"kind":"graphics-library","image":"sheet.png","sprites":[{"id":7,"name":"ship","x":0,"y":0,"width":4,"height":8}]}"#)?;
let library = Library::open(&path, None)?;
std::fs::remove_file(path)?;
let Metadata::Atlas(atlas) = library.into_metadata() else { unreachable!() };
assert_eq!(atlas.sprite(7).unwrap().name, "ship");
# Ok::<(), Box<dyn std::error::Error>>(())
```

`Library::open(path, None)` uses the source hint. `Some(format)` selects an
explicit importer; `Some(Format::Unknown)` also uses detection. PNG and unknown
formats return `Error::Asset`. Embedded NUL in an input path returns
`InvalidArgument`. Native parser failures, including allocation failure, keep
their static Asset diagnostic. No metadata or source allocation survives a
failed constructor.

`Library::from_source(source, format)` consumes an existing Source instead of
reading the file again. Failure releases that source. Parsing shares the safe
JSON import lock. `format()` records the selected format, including Aseprite.
`source()` borrows copied bytes and provenance for referenced-image paths.
`metadata()` borrows a typed Atlas, Fpg or Manifest variant, using the existing
safe inspection, conversion and animation metadata methods. All values stay on
their owner thread.

`into_metadata()` releases the source and returns independently owned metadata.
The ordinary wrappers release their native allocations once. A Library or its
Metadata can also use the existing native-backed Resource manager for process
or context ownership. Retained metadata remains usable after context shutdown.
Image creation and actual image-bound validation remain explicit; this service
does not retain textures or assign process graphics.

Rust tests load all four native library formats, use explicit/automatic formats,
reject invalid input and PNG, and inspect data after source/file/context removal.
They verify copied pivots, animation samples, FPG pixels/points and converted
manifest palette data. The runnable and owner-thread doctests pass in both
profiles. The relocated offline assets consumer uses each Library method.
Installed record/signature checks, Clippy/format and native graphical ASan/UBSan
cover the shared boundary. Rust itself is not sanitizer-instrumented.
