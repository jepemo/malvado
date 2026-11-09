# Alpha.6 reference family review

This review covers every family in [compatibility](compatibility.md), using the
[BennuGD classic and BennuGD2 catalog](inventory.md) at the pinned revisions
listed in [upstream](upstream.md). The reference names below are examples for
orientation, not a claim that their overloads or semantics match Malvado.
`family_review` checks the family rows and reference names against the catalog.

The delivered APIs are mapped in the linked inventories. **Unresolved** means
that the remaining scope, names, and contracts have not been selected. This
review does not decide those questions, promise historical compatibility, or
assign unselected work to alpha.8. Asset format compatibility is a separate
decision. Modes 7–9 and the GUI are outside 2.0 under the version plan.

| Family | Pinned reference examples | Delivered scope and remaining decision |
| --- | --- | --- |
| Process, family, signals, and lookup | `bennugd2/libmod_misc/GET_ID`, `bennugd2/libmod_misc/SIGNAL` | [Process inventory](process-inventory.md) records current APIs. Unresolved: remaining queries, signals, and reference differences. |
| Window, clock, and regions | `bennugd2/libmod_gfx/WINDOW_SET_TITLE`, `bennugd2/libmod_gfx/REGION_DEFINE` | [Platform inventory](platform-inventory.md) records current APIs. Unresolved: further window and display behavior. |
| Sprite, map, color, points, FPG | `bennugd2/libmod_gfx/FPG_LOAD`, `bennugd2/libmod_gfx/MAP_LOAD` | [Asset inventory](asset-inventory.md) records delivered atlases and the selected FPG subset. Unresolved: other map operations and format variants. |
| Drawing and persistent objects | `bennugd2/libmod_gfx/DRAW_LINE`, `bennugd1/mod_draw/DRAW_BOX` | [Drawing](drawing.md) describes current shapes, fades, and text. Unresolved: additional primitives and persistent drawing objects. |
| Text and fonts | `bennugd2/libmod_gfx/WRITE`, `bennugd2/libmod_gfx/TEXT_WIDTH` | The [text guide](lua-processes.md) and [platform inventory](platform-inventory.md) record current text and fonts. Unresolved: historical font formats and additional text operations. |
| Keyboard, mouse, controller, touch | `bennugd2/libmod_input/KEY`, `bennugd2/libmod_input/JOY_GETAXIS` | [Input inventory](input-inventory.md) records current controls. Unresolved: touch, further keys, events, and controller features. |
| Sound/music | `bennugd2/libmod_sound/SOUND_PLAY` | [Audio inventory](audio-inventory.md) records current sound and music. Unresolved: additional codecs, channels, and effects. |
| Math, random, strings | `bennugd2/libmod_misc/RAND`, `bennugd2/libmod_misc/RAND_SEED` | Lua `rand` and standard libraries are available; C has no matching engine random API. Unresolved: engine-specific math and string operations and language parity. |
| Files, directories, time | `bennugd2/libmod_misc/FOPEN`, `bennugd2/libmod_misc/GET_PREF_PATH`, `bennugd2/libmod_misc/TIME` | Lua standard libraries and a monotonic clock exist. Unresolved: save paths, packaged assets, filesystem operations, and wall-clock API. |
| Scroll/Mode 7/regions | `bennugd2/libmod_gfx/SCROLL_START`, `bennugd2/libmod_gfx/REGION_DEFINE` | Scroll translation and clipping are delivered in [drawing](drawing.md); modes 7–9 are outside 2.0. Unresolved: other 2D scroll operations. |
| Masks, pathfinding, effects | `bennugd2/libmod_gfx/COLLISION`, `bennugd2/libmod_gfx/PATH_FIND`, `bennugd1/mod_effects/GRAYSCALE` | [Collision](collision.md) covers selected geometric shapes and a static world. Unresolved: pixel masks, pathfinding, and effects. |
| Regex, sort, memory, cryptography | `bennugd2/libmod_misc/REGEX`, `bennugd2/libmod_misc/SORT`, `bennugd2/libmod_misc/MEM_ALLOC`, `bennugd2/libmod_misc/CRYPT_NEW` | Unresolved: whether engine APIs are needed beyond language libraries, and safe cross-language contracts. Raw historical pointer APIs are not implied. |
| FLIC/CD-ROM | `bennugd1/mod_flic/FLI_START`, `bennugd1/mod_cd/CD_PLAY` | Unresolved: whether to support either historical format or service; neither follows from FPG support. |
| Network/debug | `bennugd2/libmod_net/NET_OPEN`, `bennugd2/libmod_debug/TRACE` | The runner has experimental inspection hooks. Unresolved: public networking and debugging APIs and their platform scope. |
| Ads/IAP | `bennugd2/libmod_ads/ADS_INITIALIZE`, `bennugd2/libmod_iap/IAP_INITIALIZE` | These are optional platform-extension candidates in the compatibility proposal. Unresolved: whether they belong to 2.0, including service and lifecycle contracts. |

The review is a planning input. The [alpha.6 checklist](../versions/2.0.0-alpha.6.md)
still requires resolving selections, bounded alpha.8 work, semantics, and
symbol-level evidence before the version can close.
