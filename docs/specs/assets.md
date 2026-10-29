# Graphics assets: FPG, modern atlases, and ZIP packages

Status: proposed decision, not implemented support. This extends the format analysis in the plan and malvado-gui.

## Recommended decision

**Support FPG for compatibility and import; use PNG and JSON metadata as editable assets; use ZIP when it helps transport or distribute multiple files.** These are separate layers. ZIP is a container; it does not define graphics, points, animations, or IDs.

Do not create a new binary image format. A small public, versioned Malvado metadata schema is needed because control points, historical IDs, and collision profiles do not have a universal representation. Reuse existing exchange formats through adapters. Do not claim that JSON alone is one atlas standard.

## What FPG represents and what to preserve

FPG is a graphics collection from the DIV family. It is not a single spritesheet or a complete game package. The [BennuGD classic reader/writer](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/file_fpg.c) retains numeric codes, names, dimensions, graphics, and control points, and handles depth and palette variants. This helps import existing material without losing references. Do not assume all DIV/Fénix/BennuGD variants use the same layout.

The [consulted BennuGD2 implementation](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/m_fpg.c) loads FPG, while its save wrapper is commented out in that revision. Treat FPG reading and writing as separate capabilities. Do not promise export because a historical variant had it.

Current Malvado calls directories/spritesheets `fpg()` and has ZIP experiments in `malvado/map.lua`. This does not demonstrate binary FPG support. The legacy profile must distinguish that API from a real reader.

## Options

| Option | Suitable use | Benefits | Limits |
| --- | --- | --- | --- |
| Historical FPG | Reuse DIV-family graphics/projects | IDs and points known in that ecosystem | Variants, palettes, binary difficult to review; not a complete project |
| Individual PNG + JSON | Editable source and first runtime | Readable, Git-friendly, external tools, simple loading | Many files; explicit metadata contract required |
| Tool-generated PNG + JSON atlas | Exchange and render optimization | Groups regions, supports external pipelines | Different dialects; crop/rotation and points require adaptation |
| ZIP + manifest | Share libraries or distribute a game | Familiar container for multiple asset types | Needs C reader and limits; compression does not protect code |
| New proprietary binary | Optimized cache if measurements justify it | Could reduce load time | Complexity, converters, debugging, versioning; deferred |

[Aseprite exports PNG spritesheets and JSON metadata](https://www.aseprite.org/docs/cli/), including json-array/json-hash variants and animation data. Initially propose an adapter for a tested subset of PNG+JSON json-array exports. Report unsupported fields instead of ignoring them. Players do not need Aseprite or a redistributed editor.

Level maps are a separate asset. [Tiled publishes a JSON map and tileset format](https://doc.mapeditor.org/en/stable/reference/json-map-format/). Consider an importer when tilemaps are supported. Do not force a level into an FPG or promise complete Tiled compatibility in 2.0. PNG+JSON is a pipeline choice, not a claim that one modern standard replaces all DIV formats.

## Common C model for importers

Graphics library → entries with stable logical IDs → CPU image/atlas region → GPU texture. Preserve name, original size, crop offsets, pivot, control points, duration and animation sequences, filtering, and collision data when present. Define defaults for missing data. Do not invent control points from filenames without declaring that behavior.

Do not renumber FPG numeric IDs when sorting folders or packing an atlas. The compatibility `file/graph` API uses a lookup table to core assets. The modern API can use names/handles. Points use pixels in the original sprite space with a top-left origin. Apply crop offsets and transform points during rendering. Validate historical point indexes and the draw pivot separately.

For indexed FPG, preserve indexes and palette if the profile permits runtime palette changes. RGBA-only conversion can preserve the initial appearance but lose that behavior. The converter must warn and document it. Define transparency and undefined-point values with fixtures. Do not guarantee binary round trips. Import aims for pixel and supported-metadata fidelity.

Illustrative Malvado metadata, not Aseprite JSON or a frozen schema:

```json
{
  "schema": 1,
  "kind": "graphics-library",
  "sprites": [
    {
      "id": 10,
      "name": "ship",
      "image": "ship.png",
      "pivot": { "x": 16, "y": 16 },
      "points": [{ "index": 1, "name": "muzzle", "x": 16, "y": 0 }]
    }
  ]
}
```

Read size from the image and validate it on import. Adapt pivot and point conventions explicitly to the historical profile. The library schema, TOML project schema, and distribution package schema have independent versions. GUI and CLI share C validators and converters.

## Editable source and distribution

During development, use a normal folder with PNG/JSON/Lua and TOML configuration. GUI changes to a point or duration update JSON with undo/redo and atomic saving. Artist source files remain separate. Generated cache does not replace source and stays outside Git.

For export, validate/load assets and resolve references first. Generate an atlas if enabled, then package runtime and assets. Start with a validated folder export per target. A delivery ZIP can contain that folder. A runtime-mountable asset ZIP is a separate later capability with a defined manifest. Do not confuse a downloadable archive with a format the game opens.

Do not impose a new extension such as `.fpg2` or `.malvado` before it is needed. A future package extension must document its ZIP contents and schema; it must not pretend to be FPG. Android assets are inside the already packaged APK. Do not add nested ZIP by default or assume desktop paths. The asset service hides source differences, not content formats.

ZIP does not encrypt a game, and PNG is already compressed. Measure size/load time before selecting whole-package compression. Favor bounded reads and do not extract indiscriminately. Select a ZIP C library with reviewed license in P0/P3. Raylib alone is not treated as a complete package system.

## Implementation sequence

| Milestone | Capability | Acceptance criterion |
| --- | --- | --- |
| Core alpha.3 | PNG, directories/spritesheets, basic JSON library model | Sprites/demo without LÖVE; stable IDs and tested pivot |
| Core alpha.4 | Folder export and delivery ZIP | Game runs from extracted package with development references intact |
| Core alpha.5 | First pinned-variant FPG importer; basic Aseprite adapter | Selected 8-bit fixture preserves IDs/points/palette; equivalent animated spritesheet |
| GUI G1 | Browse and preview supported assets | Does not offer FPG import before a reader exists |
| GUI G3 | Edit metadata and convert supported FPG to PNG+JSON | Save/reopen and compare points, IDs, and pixels |
| 2.1+ | More FPG variants, FPG export, mountable ZIP, packing, map formats | Variant and limit matrix with dedicated tests |

Choose the first FPG variant using samples of known origin. Do not claim support for all FPG files. FPG export is optional per variant. Reject or request explicit conversion of unrepresentable data. Do not silently quantize palettes or lose alpha. The stable profile documents available loads/conversions and deferred work.

## Examples and checks

Create `fpg-import`: a small library with non-consecutive IDs, name, defined and undefined point, palette, and transparency. Import it, generate PNG+JSON, and show the same image in the runtime. Compare pixels/indexes and supported metadata. Add 16/32-bit variants only with their own fixtures.

Create `atlas-import`: Aseprite export with multiple frames/durations, crop, and pivot. Validate the subset and give clear errors for rotation or other unsupported options. Initial packing can avoid rotation. Its importer must not accept data that it will render incorrectly.

Check dimension limits, overflow, truncated sizes, duplicate IDs, and unknown versions. For ZIP, check `../` paths, absolute paths, case-variant duplicates, decompressed size, and entry count. These checks belong in the shared C reader/converter, not only the GUI dialog. FPG fixtures must be licensed for redistribution. Do not copy commercial game packages for tests.
