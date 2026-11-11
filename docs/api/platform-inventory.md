# Graphics and platform API inventory

This map covers the delivered window, drawing, image, font, fade, and platform
clock APIs. Input and audio are mapped separately in the [input](input-inventory.md)
and [audio](audio-inventory.md) inventories. The [drawing](drawing.md),
[images](images.md), [Lua process](lua-processes.md), and [C](c-contract.md)
guides define contracts, ownership, errors, and examples. This is a map of
existing APIs, not a selection of additional 2.0 features.

| Area | C | Lua | Evidence |
| --- | --- | --- | --- |
| Window | `mv_window_open`, `mv_window_close`, `mv_window_should_close`, `mv_window_set_title`, `mv_window_size` | `mv.start` or returned game table; `mv.set_title`, `mv.window_size` | `lua_start_options`, `lua_start_parity`, `visual_c_window` |
| Frame and primitives | `mv_window_begin`, `mv_window_end`, `mv_window_flush`, `mv_window_draw_box`, `mv_window_draw_circle`, `mv_window_draw_transformed_rect`, `mv_window_draw_filled_circle` | `mv.clear_screen`, `mv.set_color`, `mv.draw_box`, `mv.draw_rect`, `mv.draw_circle`, `mv.draw_fcircle`, `mv.graphics.rect`, `mv.graphics.circle` | `lua_primitives`, `visual_graphics_process`, `visual_draw_order` |
| Clipping and scroll | `mv_window_clip_begin`, `mv_window_clip_end`, `mv_window_scroll_begin`, `mv_window_scroll_end` | `mv.graphics.region`, `mv.graphics.scroll`, process `region` and `scroll` | `lua_graphics_process`, `visual_process_region`, `visual_process_scroll`, `visual_window_clip`, `visual_window_scroll` |
| Images | `mv_image_load`, `mv_image_from_rgba`, `mv_image_destroy`, `mv_image_width`, `mv_image_height`, `mv_image_draw`, `mv_image_region_valid`, `mv_image_draw_region` | `mv.load_image`, `mv.image_size`, `mv.draw_image`, `mv.draw_image_region`; managed image | `image_headless`, `visual_image_rgba`, `visual_image`, `visual_resource_lifetime` |
| Fonts and text | `mv_window_draw_text`, `mv_window_text_width`, `mv_font_load`, `mv_font_load_codepoints`, `mv_font_destroy`, `mv_font_text_width`, `mv_font_draw` | `mv.draw_text`, `mv.font`, `mv.load_font`, `mv.write`, `mv.change_text`, `mv.delete_text` | `text`, `lua_text_ownership`, `visual_font`, `visual_text_move` |
| Fade | `mv_fade_start`, `mv_fade_step`, `mv_fade_active`, `mv_fade_queue_push`, `mv_fade_queue_step`, `mv_fade_queue_active`, `mv_window_draw_fade` | `mv.fade_to`, `mv.fade_on`, `mv.fade_off`, `mv.fade_active`, `mv.fade_color` | `input`, `lua_fade`, `visual_fade` |
| Platform time | `mv_platform_time`, `mv_platform_wait`, `mv_monotonic_time` | `mv.time_seconds` uses the monotonic clock; no Lua platform wait | `platform_wait`, `headless_platform_time`, `lua_monotonic_time` |

## C platform types

| C types | Meaning and Lua relationship |
| --- | --- |
| `MvColor`, `MvFade`, `MvFadeTransition`, `MvFadeQueue` | C color and fade state. Lua uses color channels and the shared native queue. |
| `MvImage`, `MvImageRegion`, `MvImageTransform` | Reference-counted opaque C image and copied region and transform values. File loads share live normalized-path textures; release each returned reference. Lua uses managed image objects and drawing arguments. |
| `MvFont` | Opaque caller-owned C font. Lua font descriptions retain managed font resources. |

C resources must be destroyed before closing their window. Lua resource wrappers
retain their native resources. Direct C drawing is immediate; Lua drawing is
scheduled by the existing process runtime. C clip and scroll are frame state;
Lua exposes process descriptors. The headless backend cannot open a window or
load images or fonts. Asset metadata parsers remain separately available when
asset support is enabled.

`platform_api_map` checks function and type names across this map and the input
and audio maps against `platform.h`. It does not certify contracts, enum values,
Lua parity, or behavior. See the individual maps for those checks and limits.
