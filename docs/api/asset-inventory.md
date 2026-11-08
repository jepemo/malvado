# Asset API baseline for alpha.6

This inventory records the implemented asset import APIs. It is an input to the
complete 2.0 inventory. It does not select the remaining API families or certify
historical compatibility. See the [image contract](images.md) and [FPG contract](../specs/fpg-8.md)
for inputs, errors, ownership, examples, and limitations.

| Area | C | Lua | Evidence |
| --- | --- | --- | --- |
| Atlas import and lifetime | `mv_atlas_parse`, `mv_atlas_parse_aseprite`, `mv_atlas_destroy` | `mv.load_atlas`, `mv.load_aseprite`; managed atlas | `atlas`, `atlas_allocation`, `visual_image` |
| Atlas sprite lookup | `mv_atlas_count`, `mv_atlas_image`, `mv_atlas_find`, `mv_atlas_find_name` | `mv.sprite_info`, `mv.sprite_id` | `atlas`, `visual_image_graph` |
| Atlas animation and pivot metadata | `mv_atlas_animation_frame`, `mv_atlas_animation_color`, `mv_atlas_animation_repeats`, `mv_atlas_pivot_color` | `mv.animation_frame`, `mv.animation_color`, `mv.animation_repeats`, `mv.atlas_pivot_color` | `atlas`, `visual_image_graph`, `visual_image` |
| FPG import and metadata | `mv_fpg_parse`, `mv_fpg_destroy`, `mv_fpg_count`, `mv_fpg_at`, `mv_fpg_sprite`, `mv_fpg_point`, `mv_fpg_palette`, `mv_fpg_gamma` | `mv.fpg.parse` and the returned object's metadata methods | `fpg`, `fpg_allocation`, `lua_fpg` |
| FPG conversion | `mv_fpg_copy_rgba`, `mv_fpg_png_size`, `mv_fpg_copy_png`, `mv_fpg_json_size`, `mv_fpg_copy_json` | FPG object's `rgba`, `png`, `json`, and `image` methods | `fpg`, `fpg_png`, `visual_fpg_image` |
| FPG manifest | `mv_fpg_manifest_parse`, `mv_fpg_manifest_destroy`, `mv_fpg_manifest_count`, `mv_fpg_manifest_transparent_zero`, `mv_fpg_manifest_palette`, `mv_fpg_manifest_gamma`, `mv_fpg_manifest_at`, `mv_fpg_manifest_sprite`, `mv_fpg_manifest_point` | `mv.fpg.parse_manifest` and the returned object's metadata methods | `fpg`, `fpg_manifest_allocation`, `lua_fpg` |

The C importers work without Lua or a graphics backend when asset support is enabled.
Lua atlas and image creation needs a graphics runtime. FPG parsing and metadata are
available in headless Lua. Imported atlas names and paths, FPG raw metadata, and
resource lifetimes follow their respective contracts; they are not interchangeable.
