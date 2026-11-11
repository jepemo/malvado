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
