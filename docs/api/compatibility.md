# Compatibility profiles and matrix

Status: proposal. No compatibility with a new runtime is certified.

## Independent profiles

- **malvado-legacy:** preserve the current DSL, degrees, size 1.0, text keyboard keys, text, and existing examples. Record each intentional migration change.
- **malvado-2:** namespaced Lua API, safe ownership, consistent errors, and opaque C types. This is the proposed default profile.
- **bennu2-essential:** processes/signals, 2D graphics, text, input, audio, clock, and selected utilities from the pinned revision. Specify each symbol before freezing it.
- **bennu-classic-extended:** formats, scrolling, Mode 7, effects, pathfinding, and other historical functions; expand by version.
- **div/fenix:** historical comparison and explicit adaptations. There is no promise to read `.prg` or `.dcb` programs or reproduce a dedicated compiler.

## Families and work sequence

| Family | Target | Minimum example | Difference to verify |
| --- | --- | --- | --- |
| Process, family, signals, and lookup | C core + Lua binding | Parent creates children; sleep/freeze/kill-tree | Order, immediate/deferred children, collision/get_id return |
| Window, clock, and regions | C adapter + core | Scaled logical screen and two regions | Frame versus render, DPI, clipping, and shutdown |
| Sprite, map, color, points, FPG | C resources/render | Atlas, control points, mirror, alpha | Binary formats, palettes, pivots, and units |
| Drawing and persistent objects | C module | Primitives that move and are removed | Persistent ID versus one-time drawing |
| Text and fonts | C module | Fixed text, updatable score, alignment | FNT, UTF-8, `write_var`, lifetime |
| Keyboard, mouse, controller, touch | C input snapshot | Movement, click, connected controller | Scancodes, repeat, deadzones, Android |
| Sound/music | C/raylib module | WAV and music with pause | Channels, loops, codecs, and lifecycle |
| Math, random, strings | C and Lua standard library when equivalent | Fixed seed and angular movement | Millidegrees, ranges, and rounding |
| Files, directories, time | C/platform | Save a game and read assets | Writable paths, packaging, epoch |
| Scroll/Mode 7/regions | Custom C render | Tilemap camera and perspective floor | These are not direct raylib functions |
| Masks, pathfinding, effects | Custom C algorithms | Ship against contour, maze | AABB versus pixels, scale/rotation |
| Regex, sort, memory, cryptography | Per-symbol adaptation | Bounded search/sort | Bennu memory pointers are not Lua userdata |
| FLIC/CD-ROM | Historical, pending | Minimum fixture if implemented | Dedicated decoder/service or unsupported |
| Network/debug | Optional later | Local echo, process inspector | Raylib does not provide the network service |
| Ads/IAP | Optional platform extension | Event mock and SDK test | Credentials, store, lifecycle; outside essential 2.0 |

## Formats and tools

Classify FPG compatibility by variant and direction. Reading, conversion, and writing are separate capabilities. The [asset decision](../specs/assets.md) preserves IDs, points, and palettes and proposes editable PNG+JSON without removing historical `file/graph` access. The [GUI](../specs/gui.md) presents only importers/exporters that exist in C services shared with the CLI. A viewer does not certify fidelity or support for every variant.

## Per-symbol tracking record

The generated catalog uses `status=not_assessed` for all reference entries. Do not edit it manually. Create a separate decisions layer keyed by stable ID `engine/module/name/signature/return_type` (add the condition for variants), with these fields:

```json
{
  "id": "bennugd2/libmod_misc/SIGNAL/II/TYPE_INT",
  "profile": "bennu2-essential",
  "status": "planned",
  "lua_name": "malvado.signal",
  "implementation": null,
  "contract": "lua-contract.md",
  "example": "signals",
  "conformance_test": null,
  "divergences": ["Review type selector and tree handling"]
}
```

States: `not_assessed`, `specified`, `planned`, `implemented`, `verified`, `unsupported`, and `deferred`. A reason is required for the last two. Fill the matrix during implementation. The table above classifies families; it does not certify every function. For a complete profile, each included entry, overload, constant, and variable needs a decision and evidence. Add core fields as separate entries.

## API record format

Include name and aliases; origin/profile; Lua signature; C contract; arguments with units/range/default; returns/errors; ownership; affected global or local state; platform availability; runnable example; differences; and conformance test. Document historical error codes in the compatibility profile and Lua errors in the modern profile.
