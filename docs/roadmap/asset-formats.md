# Asset format extensions

Status: future proposal.

Use editable PNG and JSON assets. The current [image contract](../api/images.md)
defines the native atlas and Aseprite JSON-array subsets. The [FPG reader](../api/fpg.md)
documents the supported 8-bit variant and its conversion to PNG and JSON.

## Options to review

- Aseprite JSON-hash output, multiple slices, layers, nine-slice centers and
  more tag data. Select a small subset and define how the runtime preserves it.
- More FPG variants and writing. Treat reading and writing as separate
  decisions. Preserve IDs, control points and transparency. Document when
  indexed palettes or metadata cannot survive conversion.
- Runtime-mounted ZIP libraries and atlas packing. Define a manifest and
  limits for paths, entry counts and decompressed sizes. A delivery archive
  can contain an exported game folder today.
- Tilemap import. Evaluate [Tiled JSON](https://doc.mapeditor.org/en/stable/reference/json-map-format/)
  when there is a tilemap use case and a runtime model for it.

## Verification

Start with licensed fixtures of known origin. Compare pixels and supported
metadata, and test errors for data the importer cannot represent. Verify
IDs, crop offsets, pivots, ownership and cleanup in the supported C, Lua
and Rust paths. Check installation and exports where the feature applies.
