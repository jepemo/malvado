# Alpha.6 reference family review

This review covers every family in [compatibility](compatibility.md), using the
[BennuGD classic and BennuGD2 catalog](inventory.md) at the pinned revisions
listed in [upstream](upstream.md). The reference names below are examples for
orientation, not a claim that their overloads or semantics match Malvado.
`family_review` checks the family rows and reference names against the catalog.

The 2.0 selection retains the delivered scope below. **Disposition:** records
that selection and excludes the listed additions from 2.0. An exclusion is not
a missing alpha.6 implementation. No additional historical family is selected.
The alpha.8 multi-language and client gates remain separate deliverables.

| Family | Pinned reference examples | Delivered scope and remaining decision |
| --- | --- | --- |
| Process, family, signals, and lookup | `bennugd2/libmod_misc/GET_ID`, `bennugd2/libmod_misc/SIGNAL` | [Process inventory](process-inventory.md) records current APIs. Disposition: Delivered process inventory retained. Exclude additional historical queries, signal masks and force variants from 2.0. |
| Window, clock, and regions | `bennugd2/libmod_gfx/WINDOW_SET_TITLE`, `bennugd2/libmod_gfx/REGION_DEFINE` | [Platform inventory](platform-inventory.md) records current APIs. Disposition: Delivered window, clock and clipping retained. Exclude additional display-management APIs from 2.0. |
| Sprite, map, color, points, FPG | `bennugd2/libmod_gfx/FPG_LOAD`, `bennugd2/libmod_gfx/MAP_LOAD` | [Asset inventory](asset-inventory.md) records delivered atlases and the selected FPG subset. Disposition: Delivered PNG, atlas, Aseprite export and documented FPG subset retained. Exclude additional map operations and FPG variants from 2.0; extra Aseprite metadata remains 2.5. |
| Drawing and persistent objects | `bennugd2/libmod_gfx/DRAW_LINE`, `bennugd1/mod_draw/DRAW_BOX` | [Drawing](drawing.md) describes current shapes, fades, and text. Disposition: Delivered immediate shapes and persistent text retained. Exclude additional primitives and persistent drawing-object APIs from 2.0. |
| Text and fonts | `bennugd2/libmod_gfx/WRITE`, `bennugd2/libmod_gfx/TEXT_WIDTH` | The [text guide](lua-processes.md) and [platform inventory](platform-inventory.md) record current text and fonts. Disposition: Delivered text/font contracts retained. Exclude historical font formats and additional text operations from 2.0. |
| Keyboard, mouse, controller, touch | `bennugd2/libmod_input/KEY`, `bennugd2/libmod_input/JOY_GETAXIS` | [Input inventory](input-inventory.md) records current controls. Disposition: Delivered keyboard, mouse, controller and text input retained. Exclude touch and additional event/controller APIs from 2.0. Platform ports remain alpha.9. |
| Sound/music | `bennugd2/libmod_sound/SOUND_PLAY` | [Audio inventory](audio-inventory.md) records current sound and music. Disposition: Delivered WAV sound and music retained. Exclude additional codecs, channel mixers and effects from 2.0. |
| Math, random, strings | `bennugd2/libmod_misc/RAND`, `bennugd2/libmod_misc/RAND_SEED` | Lua `rand` and standard libraries are available; C has no matching engine random API. The [random comparison](random-comparison.md) records argument and generator differences. Disposition: Delivered Lua random and language libraries retained. Exclude an engine random-stream API and engine-specific math/string extensions from 2.0; no native random parity is claimed. |
| Files, directories, time | `bennugd2/libmod_misc/FOPEN`, `bennugd2/libmod_misc/GET_PREF_PATH`, `bennugd2/libmod_misc/TIME` | Lua standard libraries and a monotonic clock exist. Disposition: Delivered language file APIs and monotonic clock retained. Exclude engine save-path, filesystem, wall-clock and runtime ZIP APIs from 2.0. |
| Scroll/Mode 7/regions | `bennugd2/libmod_gfx/SCROLL_START`, `bennugd2/libmod_gfx/REGION_DEFINE` | Scroll translation and clipping are delivered in [drawing](drawing.md); modes 7–9 are outside 2.0. Disposition: Delivered 2D camera, region and scroll retained. Exclude additional scroll/composition/follow options from 2.0; modes 7–9 remain 2.1–2.3. |
| Masks, pathfinding, effects | `bennugd2/libmod_gfx/COLLISION`, `bennugd2/libmod_gfx/PATH_FIND`, `bennugd1/mod_effects/GRAYSCALE` | [Collision](collision.md) covers selected geometric shapes and a static world. Disposition: Delivered geometric collision and static rectangle worlds retained. Exclude masks, pathfinding, effects and visible static-world objects from 2.0. |
| Regex, sort, memory, cryptography | `bennugd2/libmod_misc/REGEX`, `bennugd2/libmod_misc/SORT`, `bennugd2/libmod_misc/MEM_ALLOC`, `bennugd2/libmod_misc/CRYPT_NEW` | Disposition: Excluded from 2.0: engine regex, sorting, raw memory and cryptography APIs. Use language libraries; no cross-language pointer contract is selected. |
| FLIC/CD-ROM | `bennugd1/mod_flic/FLI_START`, `bennugd1/mod_cd/CD_PLAY` | Disposition: Excluded from 2.0: FLIC and CD-ROM support. FPG interoperability does not require these services. |
| Network/debug | `bennugd2/libmod_net/NET_OPEN`, `bennugd2/libmod_debug/TRACE` | The runner has experimental inspection hooks. Disposition: Delivered experimental inspection retained. Exclude public networking and a new debugger protocol from 2.0. |
| Ads/IAP | `bennugd2/libmod_ads/ADS_INITIALIZE`, `bennugd2/libmod_iap/IAP_INITIALIZE` | These are optional platform-extension candidates in the compatibility proposal. Disposition: Excluded from 2.0: ads and IAP. No service, billing or extension lifecycle is selected. |

## Closure evidence

The six reference comparisons retain their pinned source and observable limits
in [process inventory](process-inventory.md#priority-comparison-with-bennugd2)
(priority, signal state, tree signals, creation hierarchy and identity) and
[random comparison](random-comparison.md). Those records link C/Lua tests for
each delivered behavior. They compare source observations, not executing the
reference engine. Random distribution and matching native streams are excluded.

| Family inventory | Contract, examples and automated evidence | Known limits and disposition |
| --- | --- | --- |
| [Process](process-inventory.md) | C contract, Lua guide, process-control and C resource/message examples; `c_process`, `c_process_handle`, `signals`, `priorities`, `natural_cascade`, `signal_relations`, `lua_rand` | Preserve the completed audit and six comparisons. No reference scheduler equivalence claimed. |
| [Graphics/platform](platform-inventory.md) | Drawing/images guides, sprites/camera examples; `visual_png_pivot`, `visual_image_cache`, `visual_camera`, `visual_resource_lifetime`, installed graphics SDK | Window resources require graphics; headless creation fails. Native callers draw explicitly; automatic process rendering is Lua. No new C rendering scheduler is selected. |
| [Collision/world](collision-inventory.md) | Collision contract, collision/world-collision examples; `collision`, `collision_allocation`, `mixed_collision`, `lua_contextual_world`, `visual_png_pivot` | Geometric shapes only; static worlds contain rectangles. Exclude mask/pathfinding/static-render extensions from 2.0. |
| [Input](input-inventory.md) | Input guide and controller/timing-game examples; `input`, `keys`, `lua_controllers`, Linux `visual_c_keys`/`visual_lua_keys` and mouse/text checks | Linux event injection needs xdotool. Controller backend checks do not certify hardware. Physical devices remain beta. Additional input APIs excluded from 2.0. |
| [Audio](audio-inventory.md) | Audio guide/demo; `audio_headless`, Linux `visual_audio`, `visual_audio_resource` | Null-device tests do not prove audible output. Physical audio remains beta. Codecs/mixers/effects excluded from 2.0. |
| [Assets](asset-inventory.md) | Images/FPG contracts, atlas/FPG/Aseprite examples; `atlas_allocation`, `lua_asset_loader`, `visual_asset_loader`, `visual_graph_selection`, relocated SDK | Metadata works without graphics; textures require a window. Native .aseprite and ZIP excluded from 2.0; extra Aseprite metadata remains 2.5. |

The inventory links contain symbol-level maps, type/constant checks and ownership
contracts. Name checks alone do not certify behavior. Changed APIs have the
A6-02–10/A6-13 acceptance evidence. Platform-specific tests remain required in
candidate CI; this decision record does not certify their execution on macOS.
