# Asset sources and content routing

Use an asset source to read a bounded file before choosing an importer. This
service is in the optional native assets library. It needs no context, Lua,
window or graphics backend. Lua `load_asset` uses it when asset support is built.

```c
#include "malvado/asset.h"

const char *error = NULL;
MvAssetSource *source = mv_asset_source_open("sheet.json", &error);
if (source) {
    size_t length;
    const char *bytes = mv_asset_source_data(source, &length);
    MvAssetFormat format = mv_asset_source_format(source);
    /* Choose the matching native importer. bytes and length are borrowed. */
    (void)bytes;
    (void)format;
    mv_asset_source_destroy(source);
}
```

Opening copies the input path and complete file into owned memory. The limit
is 64 MiB, inclusive. Files must support seeking. Empty files are valid sources.
The file is closed before detection and before the call returns. Source bytes
remain stable after the original file changes or is removed. An extra NUL
terminates the native buffer; it is not included in the reported length. Binary
source bytes can contain NUL. NULL destruction is allowed.

Opening returns NULL for invalid/empty paths, I/O errors and snapshot allocation
failure. The optional error receives a static diagnostic, or NULL on success.
No source or open FILE survives a failed call. Getters borrow storage until
source destruction. Invalid data/path getters return NULL; data errors preserve
the length output. Destruction releases the copied path and bytes once.

Detection uses content, never a filename suffix:

| Content hint | Format |
| --- | --- |
| PNG signature | `MV_ASSET_PNG` |
| FPG magic | `MV_ASSET_FPG` |
| JSON `kind: "graphics-library"` without Aseprite keys | `MV_ASSET_ATLAS` |
| JSON `kind: "malvado-fpg-8"` without Aseprite keys | `MV_ASSET_FPG_MANIFEST` |
| JSON with `meta` and `frames`, and no `kind` | `MV_ASSET_ASEPRITE` |
| Other, mixed or undetected content | `MV_ASSET_UNKNOWN` |

JSON routing is limited to 16 MiB, inclusive, and rejects embedded NUL. A failed
JSON routing parse, including allocation failure inside cJSON, gives Unknown;
the owned file snapshot remains usable. Detection is not full format validation.
Short magic-only files can have a recognized hint but fail their importer.
Native atlas, Aseprite and FPG parsers and image loaders still validate the
complete input. Explicit Lua format options continue to override the hint.

`mv_asset_source_dimensions` copies advisory Aseprite `meta.size` values. As in
the previous Lua routing, numeric values in 1..INT_MAX are truncated to integers.
Complete importer validation still rejects invalid schema values. Missing hints,
NULL outputs and aliased outputs return 0 without changing output values.

Image-path size/copy helpers join a referenced image to the source directory.
Absolute image paths pass through. Relative source paths stay relative; `..`
is preserved. These helpers do not canonicalize, check files or load images.
The size includes NUL. NULL/empty image references return 0. Invalid or small
output buffers are unchanged. Output must not overlap source or image storage.
Windows separator rules apply on Windows; automated Windows acceptance remains
in alpha.9. Native callers must serialize this service with other cJSON imports.

Lua guards each source with private GC userdata before later Lua allocations.
Import errors can release the source without leaking an open file or copied
buffer. Atlas and manifest image paths use the same native helper. Builds
without asset support retain their existing PNG-only path; JSON/FPG routing
requires the assets component. Unified resource creation, per-process asset
graph ownership and automatic animation are separate alpha.7 work.

`asset_source` checks content hints, advisory dimensions, invalid arguments,
output preservation, path joining, source removal and the exact 16/64 MiB
boundaries. `asset_source_allocation` injects snapshot and JSON allocation
failures and checks that all allocations are released. Existing Lua headless
and visual asset-loader fixtures verify importer, explicit-format, image-path
and retained-result behavior. Installed probes check every signature and the
format enum representation/values. The inventory checks cover the new header.
