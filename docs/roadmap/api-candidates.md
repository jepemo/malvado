# API candidate catalog

Status: evaluation backlog, not a Malvado API specification or a release gate.
No row below selects an implementation or a version. Apply the
[proposal reevaluation rule](README.md#promotion-into-a-release) before promotion.

## Catalog available now

The [complete pinned call inventory](../api/inventory.md) contains **1,108
function/overload declarations**: 492 BennuGD classic and 616 BennuGD2, with 629
distinct names across the two snapshots. The [machine-readable catalog](../api/catalog.json)
also preserves constants, variable blocks, conditional branches, signatures and
source links. Reuse those records instead of copying or renaming every overload
into a speculative modern API. The module index below links every call-bearing
module. Counts are declarations, not verified behavior or missing Malvado calls.

The [delivered family selection](../api/family-review.md) and
[current C inventory](../api/current-c-api.md) remain authoritative for today's
scope. A historically familiar name does not establish C/Lua/Rust equivalence.

## Reference coverage and gaps

| Reference | Evidence | Review still needed |
| --- | --- | --- |
| BennuGD classic | All 45 export headers at `d49f71891f38db9bbf08c0f6f0002007f564a03c` | Per-overload semantics, fields, conditions and behavior examples. |
| BennuGD2 | All 10 export headers at `baf8dc1b744d47dae58fc20c139dd44656907911` | Same; use as the main modern comparison, not a replacement for classic differences. |
| DIV Games Studio | [Pinned research](../api/research.md) identifies revision `77556912335d0a1b0efea855d4eb2490cf382128` | Extract original help/runtime calls, process language operations and fields; map additions/differences by source. Not yet a complete DIV catalog. |
| Fénix | [Research and source distributions](../api/research.md) identify historical material | Pin an actual source archive and hash before asserting coverage; audit calls and fields, including differences from Bennu. No complete Fénix snapshot is currently cataloged. |
| PixTudio/extensions | Research candidates | Select a revision and use case before adding exports. They are not automatically part of the catalog. |

The current inventory is complete for the cited Bennu export headers, **not** for
the union of DIV/Fénix/Bennu runtimes, language constructs and third-party DLLs.
Preserve this boundary until source-backed extraction closes the gaps. The
[manifest](../api/upstream-manifest.json) and [reproduction guide](../api/upstream.md)
record provenance; historical approximate lists are not semantic specifications.

## Capability review queue

Names below are historical comparison inputs, not promised modern signatures.
The full overload lists and conditions remain in the linked inventory.

| Family | Calls/concepts to evaluate | Modern design questions |
| --- | --- | --- |
| Processes and genealogy | GET_ID, GET_STATUS, GET_TYPE, SIGNAL, SIGNAL_ACTION, LET_ME_ALONE, EXISTS, EXIT; PROCESS/FRAME and instance fields | Useful gaps beyond existing identities, messages, signals and ownership; never introduce another scheduler. |
| Display/window | SET_MODE, SET_FPS, WINDOW_SET_TITLE, WINDOW_SET_ICON, WINDOW_GET_SIZE, WINDOW_GET_POS, WINDOW_MOVE, WINDOW_HIDE/SHOW, WINDOW_MINIMIZE/MAXIMIZE/RESTORE, WINDOW_SET_DPI_AWARE | Logical size, DPI, fullscreen, monitor and portability limits; preserve delivered controls. |
| Maps/images/points | MAP_NEW/CLONE/CLEAR/LOAD/SAVE/UNLOAD, MAP_GET_PIXEL/PUT_PIXEL, MAP_PUT/BLOCK_COPY, GRAPHIC_INFO/SET, POINT_GET/SET/GET_TOTAL, GET_REAL_POINT | Mutable versus immutable resources, CPU/GPU copies, trimmed frame coordinates; connect named points to spatial proposals. |
| Libraries/fonts/formats | FPG_NEW/ADD/DEL/LOAD/SAVE/UNLOAD, FNT_LOAD/NEW/SAVE/UNLOAD, GLYPH_GET/SET, BDF_LOAD | Explicit format/variant and read/write direction; unsupported variants must remain explicit. |
| Palette/color | RGB/RGBA/RGB_GET/RGBA_GET, PAL_LOAD/SAVE/SET/GET, PALETTE_ROLL, CONVERT_PALETTE, COLOR_FIND | Indexed versus RGBA ownership, shared palettes and shader remapping; not automatic format compatibility. |
| Drawing objects | DRAW_LINE/RECT/BOX/CIRCLE/FCIRCLE/CURVE, MOVE_DRAW, DELETE_DRAW, DRAWING_COLOR/ALPHA/Z, GET_PIXEL/PUT_PIXEL | Useful primitives and persistent IDs beyond delivered command lists; collision stays separate. |
| Text | WRITE, WRITE_VAR, WRITE_VALUE, WRITE_MOVE, WRITE_DELETE, WRITE_IN_MAP, TEXT_WIDTH/HEIGHT, WRITE_SET_RGBA | Safe updates instead of raw variable pointers, UTF-8, alignment and resource cleanup. Current persistent-text parity stays alpha.7 work. |
| Input | KEY/KEY_DOWN/KEY_UP, JOY_GETAXIS/GETBUTTON/GETHAT, JOY_IS_ATTACHED, JOY_GETACCEL; touch concepts | Delivered tick snapshots first; distinguish OS repeat, logical edges, controller mapping and mobile input. |
| Audio | SOUND_LOAD/PLAY/STOP/PAUSE/RESUME, MUSIC_LOAD/PLAY/STOP/REWIND, fade/position/volume, channel/group/panning/distance calls | Codec limits, instance state, device lifetime and a small mixing/spatial contract before effects. |
| Movement/collision/path | ADVANCE/XADVANCE, GET_ANGLE/GET_DIST, COLLISION, REGION_OUT, INTERSECT*, PATH_NEW/FIND/GETXY/WALL | Units and existing native movement/collision behavior; masks, paths and controllers are independent proposals. |
| Cameras/scroll/modes | SCROLL_START/MOVE/STOP, REGION_DEFINE, MODE7_START/STOP; sector/Mode 9 concepts | Extend delivered cameras only for real needs; use the existing graphics-mode proposal. |
| Effects/shaders | GRAYSCALE, RGBSCALE, BLUR, FILTER, blend operations, SHADER_CREATE/FREE/SET_PARAM and parameter-resource calls | Use the effect proposal; historical backend wrappers need not become public Malvado operations. |
| Files/save paths/time | FOPEN/CLOSE/READ/WRITE/SEEK, GET_PREF_PATH/BASE_PATH, DIROPEN/READ/CLOSE, GLOB, MKDIR, TIME/FTIME/GET_TIMER | Prefer language libraries where sufficient; examine portable writable paths, packaged assets and archive mounts. |
| Math/random/strings | RAND/RAND_SEED, angles/trigonometry, LERP/CLAMP, string/regex/sort functions | Avoid mirroring standard libraries; deterministic engine streams need a separate cross-language contract. |
| Network/debug/services | NET_*/socket operations, DEBUG, ads/IAP; FLIC/CD-ROM historical services | Low-priority research; portability, service ownership, obsolete formats and credentials need justification. |
| Raw memory/cryptography | ALLOC/CALLOC/REALLOC/FREE, MEM_*, CRYPT_* | Usually language/established-library responsibilities; no unsafe pointer API selected for Lua/Rust. |

Additional modern candidates include [live property editing, asset reload, flat scenes and reconciliation](live-editing.md), plus fundamental alpha/tint/blend properties,
process/camera effects, named point queries, explicit spatial links, bounded 3D
resources and editor consumers. Their provisional APIs are in the linked feature
proposals, not in the historical export inventory.

## Per-call evaluation and promotion

For each selected call, preserve its source engine/revision, module, signature,
return type and conditional context. Classify it as already covered, adapted,
standard-library alternative, candidate, declined or needing research. Record
semantics, units, ranges/defaults, errors, ownership, side effects, platform
availability, differences, a real game use case and C/Lua/Rust verification.
Use the existing [per-symbol record](../api/compatibility.md#per-symbol-tracking-record)
when a decision is made; do not change generated `not_assessed` records to imply
approval. The alpha.6 exclusion remains valid for the first 2.0 release even when
an excluded capability appears here as future research.

Before scheduling a family, choose the smallest useful subset and add one
aligned example plus required automated platform checks. A complete source-name
catalog is not a requirement to implement all those names, aliases or overloads.

## Complete call-bearing module index

Each link contains all declared calls for that module, including aliases and
overloads. Constants and variable-only modules remain in catalog.json.

| Reference/module | Declarations | Complete call list |
| --- | --- | --- |
| bennugd1/mod_blendop | 10 | [All calls](../api/inventory.md) |
| bennugd1/mod_cd | 10 | [All calls](../api/inventory.md) |
| bennugd1/mod_crypt | 6 | [All calls](../api/inventory.md) |
| bennugd1/mod_debug | 1 | [All calls](../api/inventory.md) |
| bennugd1/mod_dir | 10 | [All calls](../api/inventory.md) |
| bennugd1/mod_draw | 17 | [All calls](../api/inventory.md) |
| bennugd1/mod_effects | 4 | [All calls](../api/inventory.md) |
| bennugd1/mod_file | 22 | [All calls](../api/inventory.md) |
| bennugd1/mod_flic | 16 | [All calls](../api/inventory.md) |
| bennugd1/mod_grproc | 8 | [All calls](../api/inventory.md) |
| bennugd1/mod_joy | 34 | [All calls](../api/inventory.md) |
| bennugd1/mod_key | 1 | [All calls](../api/inventory.md) |
| bennugd1/mod_m7 | 6 | [All calls](../api/inventory.md) |
| bennugd1/mod_map | 142 | [All calls](../api/inventory.md) |
| bennugd1/mod_math | 18 | [All calls](../api/inventory.md) |
| bennugd1/mod_mathi | 18 | [All calls](../api/inventory.md) |
| bennugd1/mod_mem | 24 | [All calls](../api/inventory.md) |
| bennugd1/mod_path | 3 | [All calls](../api/inventory.md) |
| bennugd1/mod_proc | 10 | [All calls](../api/inventory.md) |
| bennugd1/mod_rand | 2 | [All calls](../api/inventory.md) |
| bennugd1/mod_regex | 4 | [All calls](../api/inventory.md) |
| bennugd1/mod_say | 2 | [All calls](../api/inventory.md) |
| bennugd1/mod_screen | 12 | [All calls](../api/inventory.md) |
| bennugd1/mod_scroll | 8 | [All calls](../api/inventory.md) |
| bennugd1/mod_sort | 5 | [All calls](../api/inventory.md) |
| bennugd1/mod_sound | 32 | [All calls](../api/inventory.md) |
| bennugd1/mod_string | 27 | [All calls](../api/inventory.md) |
| bennugd1/mod_sys | 2 | [All calls](../api/inventory.md) |
| bennugd1/mod_text | 20 | [All calls](../api/inventory.md) |
| bennugd1/mod_time | 3 | [All calls](../api/inventory.md) |
| bennugd1/mod_video | 7 | [All calls](../api/inventory.md) |
| bennugd1/mod_wm | 8 | [All calls](../api/inventory.md) |
| bennugd2/libmod_ads | 13 | [All calls](../api/inventory.md) |
| bennugd2/libmod_debug | 1 | [All calls](../api/inventory.md) |
| bennugd2/libmod_gfx | 241 | [All calls](../api/inventory.md) |
| bennugd2/libmod_iap | 7 | [All calls](../api/inventory.md) |
| bennugd2/libmod_input | 47 | [All calls](../api/inventory.md) |
| bennugd2/libmod_misc | 209 | [All calls](../api/inventory.md) |
| bennugd2/libmod_net | 10 | [All calls](../api/inventory.md) |
| bennugd2/libmod_sound | 88 | [All calls](../api/inventory.md) |
