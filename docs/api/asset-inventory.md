# Asset API baseline for alpha.6

This inventory records the implemented asset import APIs. It is an input to the
complete 2.0 inventory. It does not select the remaining API families or certify
historical compatibility. See the [image contract](images.md) and [FPG contract](../specs/fpg-8.md)
for inputs, errors, ownership, examples, and limitations.

| Area | C | Lua | Evidence |
| --- | --- | --- | --- |
| Atlas import and lifetime | `mv_atlas_parse`, `mv_atlas_parse_aseprite`, `mv_atlas_destroy` | `mv.load_atlas`, `mv.load_aseprite`; managed atlas | `atlas`, `atlas_allocation`, `visual_image` |
| Atlas sprite lookup | `mv_atlas_count`, `mv_atlas_image`, `mv_atlas_find`, `mv_atlas_find_name` | `mv.sprite_info`, `mv.sprite_id` | `atlas`, `visual_image_graph` |
| Atlas animation and pivot metadata | `mv_atlas_animation_frame_count`, `mv_atlas_animation_frame_at`, `mv_atlas_animation_frame`, `mv_atlas_animation_color`, `mv_atlas_animation_repeats`, `mv_atlas_pivot_color` | `mv.animation_frame`, `mv.animation_color`, `mv.animation_repeats`, `mv.atlas_pivot_color` | `atlas`, `visual_image_graph`, `visual_image` |
| FPG import and metadata | `mv_fpg_parse`, `mv_fpg_destroy`, `mv_fpg_count`, `mv_fpg_at`, `mv_fpg_sprite`, `mv_fpg_point`, `mv_fpg_palette`, `mv_fpg_gamma` | `mv.fpg.parse` and the returned object's metadata methods | `fpg`, `fpg_allocation`, `lua_fpg` |
| FPG conversion | `mv_fpg_copy_rgba`, `mv_fpg_png_size`, `mv_fpg_copy_png`, `mv_fpg_json_size`, `mv_fpg_copy_json` | FPG object's `rgba`, `png`, `json`, and `image` methods | `fpg`, `fpg_png`, `visual_fpg_image` |
| FPG manifest | `mv_fpg_manifest_parse`, `mv_fpg_manifest_destroy`, `mv_fpg_manifest_count`, `mv_fpg_manifest_transparent_zero`, `mv_fpg_manifest_palette`, `mv_fpg_manifest_gamma`, `mv_fpg_manifest_at`, `mv_fpg_manifest_sprite`, `mv_fpg_manifest_point` | `mv.fpg.parse_manifest` and the returned object's metadata methods | `fpg`, `fpg_manifest_allocation`, `lua_fpg` |

The C importers work without Lua or a graphics backend when asset support is enabled.
Low-level Lua atlas creation needs an image; `load_asset` also exposes headless atlas metadata. Image creation needs a graphics runtime. FPG parsing and metadata are
available in headless Lua. Imported atlas names and paths, FPG raw metadata, and
resource lifetimes follow their respective contracts; they are not interchangeable.

## Atlas types and ownership

| C types | Lua relationship and lifetime |
| --- | --- |
| `MvAtlas` | Opaque, caller-owned metadata. Lua atlas objects are managed and retain their image. |
| `MvAtlasAnimationFrame` | Copied expanded sequence entry with ID and duration; Lua animation definitions copy it. |
| `MvAtlasSprite` | A copied C record; its name is borrowed from the atlas. Lua `sprite_info` returns an independent table. |

The [atlas contract](images.md) defines validation, metadata, errors, and the
borrowed name lifetime. The [C example](../../examples/c/atlas/README.md),
`atlas`, `atlas_allocation`, and `c_atlas_example` exercise parsing and lookup.
`visual_image_graph` checks Lua sprites and resource lifetime on a graphical
backend. `asset_api_map` checks names in this table and the API table against
the public atlas header; it does not certify behavior or structure fields.

## FPG types and ownership

| C types | Lua relationship and lifetime |
| --- | --- |
| `MvFpg` | Opaque, caller-owned parsed FPG. Lua exposes a managed object, including in headless mode. |
| `MvFpgSprite` | A copied C record whose pixel pointer is borrowed from the FPG. Lua metadata and pixel strings are copies. |
| `MvFpgPoint` | A copied point value. Lua exposes point coordinates and whether a point is defined. |
| `MvFpgManifest` | Opaque, caller-owned manifest metadata; it does not load PNGs. Lua exposes a managed headless object. |
| `MvFpgManifestSprite` | A copied C manifest record. Lua exposes copied metadata. |

The [FPG contract](../specs/fpg-8.md) defines format limits, copying, errors,
and borrowed data. `fpg`, `fpg_png`, `fpg_allocation`, and
`fpg_manifest_allocation` cover the C API; `lua_fpg` and `lua_fpg_export`
cover headless Lua. The graphical `visual_fpg_image` test covers image creation.
`asset_api_map` checks the header's function and type names, not behavior or
structure fields.


## Unified Lua loading

`mv.load_asset(path, options?)`, also imported as `load_asset`, routes supported
sources to the existing image, atlas, Aseprite, FPG and manifest implementations.
It adds no native C symbols. See the [loader contract](images.md#load-an-asset-by-path),
`tests/asset_loader.lua` and `tests/asset_loader_visual.lua` for schema detection,
relative paths, metadata/GC, format/option errors and original/converted pixels.
The existing C parser, allocation, conversion and installed SDK tests remain
the native capability evidence.


`graph(id)`, `graph(library, id)`, `library:graph(id)` and `mv.graphics.graph`
select immutable static descriptors from those libraries. They reuse existing
native sprite metadata, managed images, draw transforms and collision geometry;
no native public API changed. See [static selection](images.md#select-a-static-graphic),
`tests/graph_selection.lua`, `tests/graph_selection_start.lua`, and
`tests/graph_selection_visual.lua` for context, identity, pivots, resource
retention and rendering parity. Images can still be assigned directly.
