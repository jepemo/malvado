# Inventario de funciones de referencia

Generado por `python3 docs/tools/catalog_api.py`. Una fila por sobrecarga declarada; incluye ramas condicionales y alias. No equivale a funciones implementadas en Malvado.

Las constantes y bloques globales/locales se consultan en [catalog.json](catalog.json) y en los originales de [upstream](upstream.md). Las firmas son las cadenas originales del registro C; no son firmas Lua.

| Referencia | Módulo | Sobrecargas | Constantes | Bloques global/local |
| --- | --- | ---: | ---: | ---: |
| bennugd1 | libblit | 0 | 7 | 0 |
| bennugd1 | libfont | 0 | 0 | 0 |
| bennugd1 | libgrbase | 0 | 0 | 1 |
| bennugd1 | libjoy | 0 | 9 | 0 |
| bennugd1 | libkey | 0 | 120 | 1 |
| bennugd1 | libmouse | 0 | 0 | 1 |
| bennugd1 | librender | 0 | 13 | 2 |
| bennugd1 | libscroll | 0 | 11 | 2 |
| bennugd1 | libtext | 0 | 0 | 1 |
| bennugd1 | libvideo | 0 | 36 | 1 |
| bennugd1 | libwm | 0 | 0 | 1 |
| bennugd1 | mod_blendop | 10 | 0 | 0 |
| bennugd1 | mod_cd | 10 | 5 | 1 |
| bennugd1 | mod_crypt | 6 | 3 | 0 |
| bennugd1 | mod_debug | 1 | 0 | 0 |
| bennugd1 | mod_dir | 10 | 0 | 1 |
| bennugd1 | mod_draw | 17 | 0 | 0 |
| bennugd1 | mod_effects | 4 | 12 | 0 |
| bennugd1 | mod_file | 22 | 9 | 0 |
| bennugd1 | mod_flic | 16 | 0 | 0 |
| bennugd1 | mod_grproc | 8 | 0 | 1 |
| bennugd1 | mod_joy | 34 | 0 | 0 |
| bennugd1 | mod_key | 1 | 0 | 0 |
| bennugd1 | mod_m7 | 6 | 1 | 2 |
| bennugd1 | mod_map | 142 | 14 | 0 |
| bennugd1 | mod_math | 18 | 1 | 0 |
| bennugd1 | mod_mathi | 18 | 1 | 0 |
| bennugd1 | mod_mem | 24 | 0 | 0 |
| bennugd1 | mod_mouse | 0 | 0 | 0 |
| bennugd1 | mod_path | 3 | 2 | 0 |
| bennugd1 | mod_proc | 10 | 21 | 1 |
| bennugd1 | mod_rand | 2 | 0 | 0 |
| bennugd1 | mod_regex | 4 | 0 | 1 |
| bennugd1 | mod_say | 2 | 0 | 0 |
| bennugd1 | mod_screen | 12 | 0 | 0 |
| bennugd1 | mod_scroll | 8 | 0 | 0 |
| bennugd1 | mod_sort | 5 | 0 | 0 |
| bennugd1 | mod_sound | 32 | 3 | 1 |
| bennugd1 | mod_string | 27 | 0 | 0 |
| bennugd1 | mod_sys | 2 | 2 | 0 |
| bennugd1 | mod_text | 20 | 10 | 0 |
| bennugd1 | mod_time | 3 | 0 | 0 |
| bennugd1 | mod_timers | 0 | 0 | 1 |
| bennugd1 | mod_video | 7 | 0 | 0 |
| bennugd1 | mod_wm | 8 | 0 | 0 |
| bennugd2 | libbggfx | 0 | 145 | 2 |
| bennugd2 | libbginput | 0 | 197 | 1 |
| bennugd2 | libmod_ads | 13 | 12 | 0 |
| bennugd2 | libmod_debug | 1 | 0 | 0 |
| bennugd2 | libmod_gfx | 241 | 91 | 1 |
| bennugd2 | libmod_iap | 7 | 8 | 0 |
| bennugd2 | libmod_input | 47 | 0 | 0 |
| bennugd2 | libmod_misc | 209 | 36 | 2 |
| bennugd2 | libmod_net | 10 | 7 | 0 |
| bennugd2 | libmod_sound | 88 | 27 | 1 |

## bennugd2 / libmod_ads

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `ADS_INITIALIZE` | `` | `TYPE_INT` | [L70](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_ads/libmod_ads_exports.h#L70) |
| `ADS_LOAD_INTERSTITIAL` | `S` | `TYPE_INT` | [L72](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_ads/libmod_ads_exports.h#L72) |
| `ADS_SHOW_INTERSTITIAL` | `` | `TYPE_INT` | [L73](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_ads/libmod_ads_exports.h#L73) |
| `ADS_IS_INTERSTITIAL_LOADED` | `` | `TYPE_INT` | [L74](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_ads/libmod_ads_exports.h#L74) |
| `ADS_LOAD_REWARDED` | `S` | `TYPE_INT` | [L76](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_ads/libmod_ads_exports.h#L76) |
| `ADS_SHOW_REWARDED` | `` | `TYPE_INT` | [L77](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_ads/libmod_ads_exports.h#L77) |
| `ADS_IS_REWARDED_LOADED` | `` | `TYPE_INT` | [L78](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_ads/libmod_ads_exports.h#L78) |
| `ADS_SHOW_BANNER` | `SI` | `TYPE_INT` | [L80](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_ads/libmod_ads_exports.h#L80) |
| `ADS_HIDE_BANNER` | `` | `TYPE_INT` | [L81](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_ads/libmod_ads_exports.h#L81) |
| `ADS_POLL_EVENT` | `` | `TYPE_INT` | [L83](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_ads/libmod_ads_exports.h#L83) |
| `ADS_CHECK_CONSENT` | `` | `TYPE_INT` | [L85](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_ads/libmod_ads_exports.h#L85) |
| `ADS_SHOW_CONSENT_FORM` | `` | `TYPE_INT` | [L86](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_ads/libmod_ads_exports.h#L86) |
| `ADS_SET_CHILD_DIRECTED` | `I` | `TYPE_INT` | [L87](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_ads/libmod_ads_exports.h#L87) |

## bennugd2 / libmod_debug

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `TRACE` | `I` | `TYPE_INT` | [L40](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_debug/libmod_debug_exports.h#L40) |

## bennugd2 / libmod_gfx

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `ADVANCE` | `D` | `TYPE_INT` | [L201](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L201) |
| `XADVANCE` | `ID` | `TYPE_INT` | [L202](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L202) |
| `GET_ANGLE` | `I` | `TYPE_INT` | [L204](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L204) |
| `GET_ANGLE` | `II` | `TYPE_INT` | [L205](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L205) |
| `GET_DIST` | `I` | `TYPE_DOUBLE` | [L206](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L206) |
| `GET_DIST` | `II` | `TYPE_DOUBLE` | [L207](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L207) |
| `GET_REAL_POINT` | `IDDPP` | `TYPE_INT` | [L209](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L209) |
| `GET_REAL_POINT` | `IIPP` | `TYPE_INT` | [L210](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L210) |
| `GET_REAL_POINT` | `IPP` | `TYPE_INT` | [L211](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L211) |
| `COLLISION` | `II` | `TYPE_INT` | [L213](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L213) |
| `COLLISION` | `I` | `TYPE_INT` | [L214](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L214) |
| `SCROLL_START` | `IIIIIIIII` | `TYPE_INT` | [L217](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L217) |
| `SCROLL_START` | `IIIIIII` | `TYPE_INT` | [L218](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L218) |
| `SCROLL_START` | `IIIIII` | `TYPE_INT` | [L219](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L219) |
| `SCROLL_STOP` | `I` | `TYPE_INT` | [L220](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L220) |
| `SCROLL_MOVE` | `I` | `TYPE_INT` | [L221](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L221) |
| `REGION_DEFINE` | `IIIII` | `TYPE_INT` | [L224](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L224) |
| `REGION_OUT` | `II` | `TYPE_INT` | [L225](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L225) |
| `SCREEN_GET` | `` | `TYPE_INT` | [L228](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L228) |
| `RGB` | `BBB` | `TYPE_INT` | [L230](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L230) |
| `RGB` | `IIBBB` | `TYPE_INT` | [L231](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L231) |
| `RGBA` | `BBBB` | `TYPE_INT` | [L232](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L232) |
| `RGBA` | `IIBBBB` | `TYPE_INT` | [L233](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L233) |
| `RGB_GET` | `IPPP` | `TYPE_INT` | [L234](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L234) |
| `RGB_GET` | `IIIPPP` | `TYPE_INT` | [L235](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L235) |
| `RGBA_GET` | `IPPPP` | `TYPE_INT` | [L236](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L236) |
| `RGBA_GET` | `IIIPPPP` | `TYPE_INT` | [L237](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L237) |
| `FADE` | `IIIII` | `TYPE_INT` | [L239](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L239) |
| `FADE` | `IIIIII` | `TYPE_INT` | [L240](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L240) |
| `FADE_ON` | `I` | `TYPE_INT` | [L241](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L241) |
| `FADE_ON` | `II` | `TYPE_INT` | [L242](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L242) |
| `FADE_OFF` | `I` | `TYPE_INT` | [L243](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L243) |
| `FADE_OFF` | `II` | `TYPE_INT` | [L244](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L244) |
| `SET_MODE` | `II` | `TYPE_INT` | [L247](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L247) |
| `SET_MODE` | `III` | `TYPE_INT` | [L248](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L248) |
| `SET_FPS` | `II` | `TYPE_INT` | [L249](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L249) |
| `WINDOW_SET_TITLE` | `S` | `TYPE_INT` | [L251](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L251) |
| `WINDOW_SET_ICON` | `II` | `TYPE_INT` | [L252](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L252) |
| `WINDOW_MOVE` | `II` | `TYPE_INT` | [L253](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L253) |
| `WINDOW_SET_POS` | `II` | `TYPE_INT` | [L254](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L254) |
| `WINDOW_GET_POS` | `PP` | `TYPE_INT` | [L255](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L255) |
| `WINDOW_GET_SIZE` | `PPPPPP` | `TYPE_INT` | [L256](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L256) |
| `DESKTOP_GET_SIZE` | `PP` | `TYPE_INT` | [L257](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L257) |
| `WINDOW_HIDE` | `` | `TYPE_INT` | [L259](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L259) |
| `WINDOW_SHOW` | `` | `TYPE_INT` | [L260](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L260) |
| `WINDOW_MINIMIZE` | `` | `TYPE_INT` | [L261](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L261) |
| `WINDOW_MAXIMIZE` | `` | `TYPE_INT` | [L262](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L262) |
| `WINDOW_RAISE` | `` | `TYPE_INT` | [L263](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L263) |
| `WINDOW_RESTORE` | `` | `TYPE_INT` | [L264](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L264) |
| `DISPLAY_GET_ORIENTATION` | `` | `TYPE_INT` | [L266](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L266) |
| `WINDOW_SET_DPI_AWARE` | `` | `TYPE_INT` | [L267](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L267) |
| `WRITE` | `IIIIS` | `TYPE_INT` | [L271](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L271) |
| `WRITE` | `IIIIIS` | `TYPE_INT` | [L272](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L272) |
| `WRITE_MOVE` | `III` | `TYPE_INT` | [L273](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L273) |
| `WRITE_MOVE` | `IIII` | `TYPE_INT` | [L274](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L274) |
| `WRITE_DELETE` | `I` | `TYPE_INT` | [L275](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L275) |
| `WRITE_IN_MAP` | `ISI` | `TYPE_INT` | [L276](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L276) |
| `WRITE_IN_MAP` | `IIIIISI` | `TYPE_INT` | [L277](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L277) |
| `TEXT_WIDTH` | `IS` | `TYPE_INT` | [L279](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L279) |
| `TEXT_HEIGHT` | `IS` | `TYPE_INT` | [L280](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L280) |
| `WRITE_VAR` | `IIIIV++` | `TYPE_INT` | [L283](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L283) |
| `WRITE_VAR` | `IIIIIV++` | `TYPE_INT` | [L284](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L284) |
| `WRITE_VALUE` | `IIIIIP` | `TYPE_INT` | [L285](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L285) |
| `WRITE_VALUE` | `IIIIIIP` | `TYPE_INT` | [L286](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L286) |
| `WRITE_GET_RGBA` | `IPPPP` | `TYPE_INT` | [L288](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L288) |
| `WRITE_SET_RGBA` | `IBBBB` | `TYPE_INT` | [L289](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L289) |
| `MAP_BLOCK_COPY` | `IIIIIIIIIII` | `TYPE_INT` | [L292](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L292) |
| `MAP_BLOCK_COPY` | `IIIIIIIIIIIBBBB` | `TYPE_INT` | [L293](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L293) |
| `MAP_BLOCK_COPY` | `IIIIIIIIIIIBBBBI` | `TYPE_INT` | [L294](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L294) |
| `MAP_BLOCK_COPY` | `IIIIIIIIIIIBBBBIIIIIII` | `TYPE_INT` | [L295](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L295) |
| `MAP_BLOCK_COPY` | `IIIIIIIIII` | `TYPE_INT` | [L296](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L296) |
| `MAP_PUT` | `IIIIII` | `TYPE_INT` | [L297](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L297) |
| `MAP_PUT` | `IIIIIIIIIIBBBB` | `TYPE_INT` | [L298](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L298) |
| `MAP_PUT` | `IIIIIIIIIIBBBBI` | `TYPE_INT` | [L299](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L299) |
| `MAP_PUT` | `IIIIIIIIIIBBBBIIIIIII` | `TYPE_INT` | [L300](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L300) |
| `MAP_NEW` | `II` | `TYPE_INT` | [L301](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L301) |
| `MAP_NEW` | `III` | `TYPE_INT` | [L302](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L302) |
| `MAP_CLEAR` | `II` | `TYPE_INT` | [L303](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L303) |
| `MAP_CLEAR` | `III` | `TYPE_INT` | [L304](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L304) |
| `MAP_CLEAR` | `IIIIII` | `TYPE_INT` | [L305](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L305) |
| `MAP_CLEAR` | `IIIIIII` | `TYPE_INT` | [L306](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L306) |
| `MAP_CLONE` | `II` | `TYPE_INT` | [L307](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L307) |
| `MAP_NAME` | `II` | `TYPE_STRING` | [L308](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L308) |
| `MAP_SET_NAME` | `IIS` | `TYPE_INT` | [L309](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L309) |
| `MAP_EXISTS` | `II` | `TYPE_INT` | [L310](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L310) |
| `MAP_DEL` | `II` | `TYPE_INT` | [L311](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L311) |
| `MAP_UNLOAD` | `II` | `TYPE_INT` | [L312](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L312) |
| `MAP_LOAD` | `S` | `TYPE_INT` | [L313](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L313) |
| `MAP_LOAD` | `SP` | `TYPE_INT` | [L314](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L314) |
| `MAP_SAVE` | `IIS` | `TYPE_INT` | [L315](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L315) |
| `MAP_GET_PIXEL` | `IIII` | `TYPE_INT` | [L317](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L317) |
| `MAP_PUT_PIXEL` | `IIIII` | `TYPE_INT` | [L318](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L318) |
| `FPG_ADD` | `IIII` | `TYPE_INT` | [L321](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L321) |
| `FPG_NEW` | `` | `TYPE_INT` | [L322](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L322) |
| `FPG_EXISTS` | `I` | `TYPE_INT` | [L323](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L323) |
| `FPG_LOAD` | `S` | `TYPE_INT` | [L324](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L324) |
| `FPG_LOAD` | `SP` | `TYPE_INT` | [L325](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L325) |
| `FPG_DEL` | `I` | `TYPE_INT` | [L327](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L327) |
| `FPG_UNLOAD` | `I` | `TYPE_INT` | [L328](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L328) |
| `MAP_INFO_SET` | `IIII` | `TYPE_INT` | [L331](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L331) |
| `MAP_INFO` | `IIII` | `TYPE_INT` | [L332](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L332) |
| `MAP_INFO` | `III` | `TYPE_INT` | [L333](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L333) |
| `GRAPHIC_SET` | `IIII` | `TYPE_INT` | [L335](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L335) |
| `GRAPHIC_INFO` | `III` | `TYPE_INT` | [L336](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L336) |
| `POINT_GET` | `IIIPP` | `TYPE_INT` | [L339](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L339) |
| `POINT_SET` | `IIIII` | `TYPE_INT` | [L340](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L340) |
| `CENTER_SET` | `IIII` | `TYPE_INT` | [L341](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L341) |
| `POINT_GET_TOTAL` | `II` | `TYPE_INT` | [L342](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L342) |
| `BOX_SET` | `IIIIIIII` | `TYPE_INT` | [L345](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L345) |
| `BOX_SET` | `IIIIII` | `TYPE_INT` | [L346](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L346) |
| `BOX_GET` | `IIIPPPPP` | `TYPE_INT` | [L347](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L347) |
| `BOX_GET` | `IPPPPP` | `TYPE_INT` | [L348](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L348) |
| `BOX_GET_BY_POS` | `IIIPPPPPP` | `TYPE_INT` | [L349](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L349) |
| `BOX_GET_BY_POS` | `IPPPPPP` | `TYPE_INT` | [L350](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L350) |
| `BOX_REMOVE` | `III` | `TYPE_INT` | [L351](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L351) |
| `BOX_REMOVE` | `I` | `TYPE_INT` | [L352](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L352) |
| `BOX_GET_REAL_VERTEX` | `IIIIPP` | `TYPE_INT` | [L353](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L353) |
| `BOX_GET_REAL_VERTEX` | `IIIPP` | `TYPE_INT` | [L354](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L354) |
| `BOX_GET_REAL_VERTEX` | `IPP` | `TYPE_INT` | [L355](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L355) |
| `FNT_NEW` | `` | `TYPE_INT` | [L358](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L358) |
| `FNT_NEW` | `I` | `TYPE_INT` | [L359](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L359) |
| `FNT_NEW` | `IIIIIIII` | `TYPE_INT` | [L360](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L360) |
| `FNT_NEW` | `IIIIIIIIS` | `TYPE_INT` | [L361](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L361) |
| `FNT_NEW` | `IIIIIIIIIIII` | `TYPE_INT` | [L362](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L362) |
| `FNT_NEW` | `IIIIIIIIIIIIS` | `TYPE_INT` | [L363](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L363) |
| `FNT_NEW` | `IIIIIIIIIIIII` | `TYPE_INT` | [L364](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L364) |
| `FNT_NEW` | `IIIIIIIISIIIII` | `TYPE_INT` | [L365](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L365) |
| `FNT_NEW` | `IIIIIIIIIIIIIIIII` | `TYPE_INT` | [L366](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L366) |
| `FNT_NEW` | `IIIIIIIIIIIISIIIII` | `TYPE_INT` | [L367](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L367) |
| `FNT_LOAD` | `S` | `TYPE_INT` | [L368](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L368) |
| `FNT_LOAD` | `SP` | `TYPE_INT` | [L369](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L369) |
| `FNT_UNLOAD` | `I` | `TYPE_INT` | [L370](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L370) |
| `GLYPH_GET` | `II` | `TYPE_INT` | [L374](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L374) |
| `GLYPH_SET` | `IIII` | `TYPE_INT` | [L375](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L375) |
| `DRAWING_COLOR` | `I` | `TYPE_INT` | [L379](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L379) |
| `DRAWING_COLOR` | `II` | `TYPE_INT` | [L380](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L380) |
| `DRAWING_RGBA` | `BBBB` | `TYPE_INT` | [L381](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L381) |
| `DRAWING_RGBA` | `IBBBB` | `TYPE_INT` | [L382](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L382) |
| `DRAWING_BLENDMODE` | `I` | `TYPE_INT` | [L383](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L383) |
| `DRAWING_BLENDMODE` | `II` | `TYPE_INT` | [L384](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L384) |
| `DRAWING_BLENDMODE` | `IIIIII` | `TYPE_INT` | [L385](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L385) |
| `DRAWING_BLENDMODE` | `IIIIIII` | `TYPE_INT` | [L386](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L386) |
| `DRAWING_Z` | `` | `TYPE_INT` | [L388](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L388) |
| `DRAWING_Z` | `I` | `TYPE_INT` | [L389](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L389) |
| `DRAWING_Z` | `II` | `TYPE_INT` | [L390](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L390) |
| `DRAWING_MAP` | `II` | `TYPE_INT` | [L391](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L391) |
| `DRAW_DELETE` | `I` | `TYPE_INT` | [L393](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L393) |
| `DRAW_MOVE` | `III` | `TYPE_INT` | [L395](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L395) |
| `DRAW_COORDS` | `IPPP` | `TYPE_INT` | [L396](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L396) |
| `DRAW_POINT` | `II` | `TYPE_INT` | [L398](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L398) |
| `DRAW_POINT` | `III` | `TYPE_INT` | [L399](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L399) |
| `DRAW_LINE` | `IIII` | `TYPE_INT` | [L401](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L401) |
| `DRAW_LINE` | `IIIII` | `TYPE_INT` | [L402](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L402) |
| `DRAW_RECTANGLE` | `IIII` | `TYPE_INT` | [L404](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L404) |
| `DRAW_RECTANGLE` | `IIIII` | `TYPE_INT` | [L405](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L405) |
| `DRAW_RECTANGLE_FILLED` | `IIII` | `TYPE_INT` | [L407](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L407) |
| `DRAW_RECTANGLE_FILLED` | `IIIII` | `TYPE_INT` | [L408](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L408) |
| `DRAW_POINTS` | `IP` | `TYPE_INT` | [L411](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L411) |
| `DRAW_POINTS` | `IPI` | `TYPE_INT` | [L412](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L412) |
| `DRAW_LINES` | `IP` | `TYPE_INT` | [L414](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L414) |
| `DRAW_LINES` | `IPI` | `TYPE_INT` | [L415](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L415) |
| `DRAW_RECTANGLES` | `IP` | `TYPE_INT` | [L417](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L417) |
| `DRAW_RECTANGLES` | `IPI` | `TYPE_INT` | [L418](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L418) |
| `DRAW_RECTANGLES_FILLED` | `IP` | `TYPE_INT` | [L420](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L420) |
| `DRAW_RECTANGLES_FILLED` | `IPI` | `TYPE_INT` | [L421](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L421) |
| `DRAW_CIRCLE` | `III` | `TYPE_INT` | [L424](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L424) |
| `DRAW_CIRCLE` | `IIII` | `TYPE_INT` | [L425](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L425) |
| `DRAW_CIRCLE_FILLED` | `III` | `TYPE_INT` | [L426](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L426) |
| `DRAW_CIRCLE_FILLED` | `IIII` | `TYPE_INT` | [L427](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L427) |
| `DRAW_CURVE` | `IIIIIIIII` | `TYPE_INT` | [L429](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L429) |
| `DRAW_CURVE` | `IIIIIIIIII` | `TYPE_INT` | [L430](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L430) |
| `DRAW_ARC` | `IIIII` | `TYPE_INT` | [L433](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L433) |
| `DRAW_ARC` | `IIIIII` | `TYPE_INT` | [L434](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L434) |
| `DRAW_ARC_FILLED` | `IIIII` | `TYPE_INT` | [L435](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L435) |
| `DRAW_ARC_FILLED` | `IIIIII` | `TYPE_INT` | [L436](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L436) |
| `DRAW_ELLIPSE` | `IIIII` | `TYPE_INT` | [L438](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L438) |
| `DRAW_ELLIPSE` | `IIIIII` | `TYPE_INT` | [L439](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L439) |
| `DRAW_ELLIPSE_FILLED` | `IIIII` | `TYPE_INT` | [L440](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L440) |
| `DRAW_ELLIPSE_FILLED` | `IIIIII` | `TYPE_INT` | [L441](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L441) |
| `DRAW_SECTOR` | `IIIIII` | `TYPE_INT` | [L443](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L443) |
| `DRAW_SECTOR` | `IIIIIII` | `TYPE_INT` | [L444](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L444) |
| `DRAW_SECTOR_FILLED` | `IIIIII` | `TYPE_INT` | [L445](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L445) |
| `DRAW_SECTOR_FILLED` | `IIIIIII` | `TYPE_INT` | [L446](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L446) |
| `DRAW_TRIANGLE` | `IIIIII` | `TYPE_INT` | [L448](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L448) |
| `DRAW_TRIANGLE` | `IIIIIII` | `TYPE_INT` | [L449](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L449) |
| `DRAW_TRIANGLE_FILLED` | `IIIIII` | `TYPE_INT` | [L450](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L450) |
| `DRAW_TRIANGLE_FILLED` | `IIIIIII` | `TYPE_INT` | [L451](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L451) |
| `DRAW_RECTANGLE_ROUND` | `IIIII` | `TYPE_INT` | [L453](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L453) |
| `DRAW_RECTANGLE_ROUND` | `IIIIII` | `TYPE_INT` | [L454](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L454) |
| `DRAW_RECTANGLE_ROUND_FILLED` | `IIIII` | `TYPE_INT` | [L455](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L455) |
| `DRAW_RECTANGLE_ROUND_FILLED` | `IIIIII` | `TYPE_INT` | [L456](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L456) |
| `DRAW_POLYGON` | `IP` | `TYPE_INT` | [L458](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L458) |
| `DRAW_POLYGON` | `IPI` | `TYPE_INT` | [L459](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L459) |
| `DRAW_POLYGON_FILLED` | `IP` | `TYPE_INT` | [L460](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L460) |
| `DRAW_POLYGON_FILLED` | `IPI` | `TYPE_INT` | [L461](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L461) |
| `DRAW_POLYLINE` | `IPI` | `TYPE_INT` | [L463](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L463) |
| `DRAW_POLYLINE` | `IPII` | `TYPE_INT` | [L464](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L464) |
| `DRAW_SET_THICKNESS` | `F` | `TYPE_FLOAT` | [L466](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L466) |
| `DRAW_SET_THICKNESS` | `IF` | `TYPE_FLOAT` | [L467](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L467) |
| `DRAW_GET_THICKNESS` | `` | `TYPE_FLOAT` | [L468](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L468) |
| `DRAW_GET_THICKNESS` | `I` | `TYPE_FLOAT` | [L469](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L469) |
| `PATH_NEW` | `II` | `TYPE_POINTER` | [L474](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L474) |
| `PATH_DESTROY` | `P` | `TYPE_INT` | [L475](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L475) |
| `PATH_FIND` | `PIIIII` | `TYPE_POINTER` | [L476](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L476) |
| `PATH_FIND` | `PIIIIIII` | `TYPE_POINTER` | [L477](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L477) |
| `PATH_FREE_RESULTS` | `P` | `TYPE_INT` | [L478](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L478) |
| `TEXTURE_SET_QUALITY` | `I` | `TYPE_INT` | [L480](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L480) |
| `MAP_SET_PALETTE` | `IIIIP` | `TYPE_INT` | [L482](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L482) |
| `MAP_GET_PALETTE` | `IIIIP` | `TYPE_INT` | [L483](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L483) |
| `SHADER_GET_LANGUAGE` | `` | `TYPE_INT` | [L487](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L487) |
| `SHADER_GET_MIN_VERSION` | `` | `TYPE_INT` | [L488](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L488) |
| `SHADER_GET_MAX_VERSION` | `` | `TYPE_INT` | [L489](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L489) |
| `SHADER_CREATE` | `SS` | `TYPE_POINTER` | [L491](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L491) |
| `SHADER_FREE` | `P` | `TYPE_INT` | [L492](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L492) |
| `SHADER_GETATTRIBUTELOCATION` | `PS` | `TYPE_INT` | [L495](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L495) |
| `SHADER_GET_PARAM_LOCATION` | `PS` | `TYPE_INT` | [L497](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L497) |
| `SHADER_CREATE_PARAMS` | `I` | `TYPE_POINTER` | [L499](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L499) |
| `SHADER_FREE_PARAMS` | `P` | `TYPE_INT` | [L501](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L501) |
| `SHADER_SET_PARAM` | `PIII` | `TYPE_INT` | [L503](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L503) |
| `SHADER_SET_PARAM` | `PIIF` | `TYPE_INT` | [L504](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L504) |
| `SHADER_SET_PARAM` | `PIIIII` | `TYPE_INT` | [L505](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L505) |
| `SHADER_SET_PARAM` | `PIIIP` | `TYPE_INT` | [L506](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L506) |
| `SHADER_SET_PARAM` | `PIIPIIII` | `TYPE_INT` | [L507](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L507) |
| `SHADER_GET_PARAM` | `PIPP` | `TYPE_INT` | [L509](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L509) |
| `SHADER_GET_PARAM` | `PIPPP` | `TYPE_INT` | [L510](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L510) |
| `SHADER_GET_PARAM` | `PIPPPPPP` | `TYPE_INT` | [L511](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L511) |
| `MEDIA_LOAD` | `SP` | `TYPE_POINTER` | [L515](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L515) |
| `MEDIA_LOAD` | `SPI` | `TYPE_POINTER` | [L516](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L516) |
| `MEDIA_UNLOAD` | `P` | `TYPE_INT` | [L517](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L517) |
| `MEDIA_PLAY` | `P` | `TYPE_INT` | [L518](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L518) |
| `MEDIA_PAUSE` | `P` | `TYPE_INT` | [L519](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L519) |
| `MEDIA_PAUSE` | `PI` | `TYPE_INT` | [L520](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L520) |
| `MEDIA_RESUME` | `P` | `TYPE_INT` | [L521](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L521) |
| `MEDIA_STOP` | `P` | `TYPE_INT` | [L522](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L522) |
| `MEDIA_GET_TIME` | `P` | `TYPE_INT` | [L523](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L523) |
| `MEDIA_GET_DURATION` | `P` | `TYPE_INT` | [L524](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L524) |
| `MEDIA_GET_STATUS` | `P` | `TYPE_INT` | [L525](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L525) |
| `MEDIA_GET_MUTE` | `P` | `TYPE_INT` | [L526](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L526) |
| `MEDIA_SET_MUTE` | `PI` | `TYPE_INT` | [L527](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L527) |
| `MEDIA_GET_VOLUME` | `P` | `TYPE_INT` | [L528](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L528) |
| `MEDIA_SET_VOLUME` | `PI` | `TYPE_INT` | [L529](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/libmod_gfx_exports.h#L529) |

## bennugd2 / libmod_iap

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `IAP_INITIALIZE` | `` | `TYPE_INT` | [L26](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_iap/libmod_iap_exports.h#L26) |
| `IAP_PURCHASE` | `S` | `TYPE_INT` | [L27](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_iap/libmod_iap_exports.h#L27) |
| `IAP_CONSUME` | `S` | `TYPE_INT` | [L28](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_iap/libmod_iap_exports.h#L28) |
| `IAP_GET_PRICE` | `S` | `TYPE_STRING` | [L29](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_iap/libmod_iap_exports.h#L29) |
| `IAP_POLL_EVENT` | `` | `TYPE_INT` | [L30](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_iap/libmod_iap_exports.h#L30) |
| `IAP_GET_LAST_PURCHASE_SKU` | `` | `TYPE_STRING` | [L31](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_iap/libmod_iap_exports.h#L31) |
| `IAP_GET_LAST_PURCHASE_TOKEN` | `` | `TYPE_STRING` | [L32](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_iap/libmod_iap_exports.h#L32) |

## bennugd2 / libmod_input

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `KEY` | `I` | `TYPE_INT` | [L43](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L43) |
| `KEYUP` | `I` | `TYPE_INT` | [L44](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L44) |
| `KEY_UP` | `I` | `TYPE_INT` | [L45](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L45) |
| `KEYDOWN` | `I` | `TYPE_INT` | [L46](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L46) |
| `KEY_DOWN` | `I` | `TYPE_INT` | [L47](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L47) |
| `JOY_NAME` | `` | `TYPE_STRING` | [L54](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L54) |
| `JOY_NAME` | `I` | `TYPE_STRING` | [L55](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L55) |
| `JOY_SELECT` | `I` | `TYPE_INT` | [L59](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L59) |
| `JOY_NUMBER` | `` | `TYPE_INT` | [L63](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L63) |
| `JOY_NUMJOYSTICKS` | `` | `TYPE_INT` | [L64](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L64) |
| `JOY_BUTTONS` | `` | `TYPE_INT` | [L66](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L66) |
| `JOY_BUTTONS` | `I` | `TYPE_INT` | [L67](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L67) |
| `JOY_NUMBUTTONS` | `` | `TYPE_INT` | [L68](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L68) |
| `JOY_NUMBUTTONS` | `I` | `TYPE_INT` | [L69](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L69) |
| `JOY_AXES` | `` | `TYPE_INT` | [L71](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L71) |
| `JOY_AXES` | `I` | `TYPE_INT` | [L72](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L72) |
| `JOY_NUMAXES` | `` | `TYPE_INT` | [L73](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L73) |
| `JOY_NUMAXES` | `I` | `TYPE_INT` | [L74](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L74) |
| `JOY_NUMHATS` | `` | `TYPE_INT` | [L76](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L76) |
| `JOY_NUMHATS` | `I` | `TYPE_INT` | [L77](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L77) |
| `JOY_NUMBALLS` | `` | `TYPE_INT` | [L79](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L79) |
| `JOY_NUMBALLS` | `I` | `TYPE_INT` | [L80](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L80) |
| `JOY_GETAXIS` | `I` | `TYPE_INT` | [L84](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L84) |
| `JOY_GETAXIS` | `II` | `TYPE_INT` | [L85](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L85) |
| `JOY_GETPOSITION` | `I` | `TYPE_INT` | [L86](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L86) |
| `JOY_GETPOSITION` | `II` | `TYPE_INT` | [L87](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L87) |
| `JOY_GETBUTTON` | `I` | `TYPE_INT` | [L89](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L89) |
| `JOY_GETBUTTON` | `II` | `TYPE_INT` | [L90](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L90) |
| `JOY_GETHAT` | `I` | `TYPE_INT` | [L92](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L92) |
| `JOY_GETHAT` | `II` | `TYPE_INT` | [L93](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L93) |
| `JOY_GETBALL` | `IPP` | `TYPE_INT` | [L95](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L95) |
| `JOY_GETBALL` | `IIPP` | `TYPE_INT` | [L96](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L96) |
| `JOY_GETACCEL` | `PPP` | `TYPE_INT` | [L98](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L98) |
| `JOY_GETACCEL` | `IPPP` | `TYPE_INT` | [L99](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L99) |
| `JOY_IS_ATTACHED` | `` | `TYPE_INT` | [L103](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L103) |
| `JOY_IS_ATTACHED` | `I` | `TYPE_INT` | [L104](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L104) |
| `JOY_QUERY` | `I` | `TYPE_INT` | [L106](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L106) |
| `JOY_QUERY` | `II` | `TYPE_INT` | [L107](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L107) |
| `JOY_QUERY` | `III` | `TYPE_INT` | [L108](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L108) |
| `JOY_SET` | `IIII` | `TYPE_INT` | [L109](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L109) |
| `JOY_SET` | `IIIII` | `TYPE_INT` | [L110](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L110) |
| `NUMBER_JOY` | `` | `TYPE_INT` | [L114](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L114) |
| `SELECT_JOY` | `I` | `TYPE_INT` | [L115](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L115) |
| `GET_JOY_BUTTON` | `I` | `TYPE_INT` | [L116](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L116) |
| `GET_JOY_BUTTON` | `II` | `TYPE_INT` | [L117](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L117) |
| `GET_JOY_POSITION` | `I` | `TYPE_INT` | [L118](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L118) |
| `GET_JOY_POSITION` | `II` | `TYPE_INT` | [L119](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_input/libmod_input_exports.h#L119) |

## bennugd2 / libmod_misc

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `CRYPT_NEW` | `IP` | `TYPE_POINTER` | [L144](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L144) |
| `CRYPT_DEL` | `P` | `TYPE_INT` | [L145](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L145) |
| `CRYPT_ENCRYPT` | `PPPI` | `TYPE_INT` | [L146](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L146) |
| `CRYPT_DECRYPT` | `PPPI` | `TYPE_INT` | [L147](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L147) |
| `CRYPT_ENCRYPT` | `IPPPI` | `TYPE_INT` | [L148](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L148) |
| `CRYPT_DECRYPT` | `IPPPI` | `TYPE_INT` | [L149](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L149) |
| `CD` | `` | `TYPE_STRING` | [L153](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L153) |
| `CD` | `S` | `TYPE_INT` | [L154](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L154) |
| `CHDIR` | `S` | `TYPE_INT` | [L155](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L155) |
| `MKDIR` | `S` | `TYPE_INT` | [L156](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L156) |
| `RMDIR` | `S` | `TYPE_INT` | [L157](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L157) |
| `GLOB` | `S` | `TYPE_STRING` | [L158](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L158) |
| `RM` | `S` | `TYPE_INT` | [L159](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L159) |
| `DIROPEN` | `S` | `TYPE_INT` | [L161](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L161) |
| `DIRCLOSE` | `I` | `TYPE_INT` | [L162](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L162) |
| `DIRREAD` | `I` | `TYPE_STRING` | [L163](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L163) |
| `GET_BASE_PATH` | `` | `TYPE_STRING` | [L166](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L166) |
| `GET_PREF_PATH` | `SS` | `TYPE_STRING` | [L167](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L167) |
| `SAVE` | `SV++` | `TYPE_INT` | [L170](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L170) |
| `LOAD` | `SV++` | `TYPE_INT` | [L171](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L171) |
| `FILE` | `S` | `TYPE_STRING` | [L173](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L173) |
| `FOPEN` | `SI` | `TYPE_INT` | [L175](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L175) |
| `FCLOSE` | `I` | `TYPE_INT` | [L176](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L176) |
| `FREAD` | `IV++` | `TYPE_INT` | [L177](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L177) |
| `FREAD` | `PII` | `TYPE_INT` | [L178](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L178) |
| `FWRITE` | `IV++` | `TYPE_INT` | [L179](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L179) |
| `FWRITE` | `PII` | `TYPE_INT` | [L180](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L180) |
| `FSEEK` | `III` | `TYPE_INT` | [L181](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L181) |
| `FREWIND` | `I` | `TYPE_UNDEFINED` | [L182](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L182) |
| `FTELL` | `I` | `TYPE_INT` | [L183](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L183) |
| `FFLUSH` | `I` | `TYPE_INT` | [L184](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L184) |
| `FPUTS` | `IS` | `TYPE_INT` | [L185](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L185) |
| `FGETS` | `I` | `TYPE_STRING` | [L186](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L186) |
| `FEOF` | `I` | `TYPE_INT` | [L187](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L187) |
| `FLENGTH` | `I` | `TYPE_INT` | [L189](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L189) |
| `FEXISTS` | `S` | `TYPE_INT` | [L190](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L190) |
| `FREMOVE` | `S` | `TYPE_INT` | [L191](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L191) |
| `FMOVE` | `SS` | `TYPE_INT` | [L192](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L192) |
| `MAX` | `DD` | `TYPE_DOUBLE` | [L195](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L195) |
| `MIN` | `DD` | `TYPE_DOUBLE` | [L196](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L196) |
| `SGN` | `D` | `TYPE_DOUBLE` | [L197](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L197) |
| `SGN` | `I` | `TYPE_INT` | [L198](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L198) |
| `ROUND` | `D` | `TYPE_DOUBLE` | [L199](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L199) |
| `FLOOR` | `D` | `TYPE_DOUBLE` | [L200](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L200) |
| `CEIL` | `D` | `TYPE_DOUBLE` | [L201](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L201) |
| `TRUNC` | `D` | `TYPE_DOUBLE` | [L202](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L202) |
| `FRAC` | `D` | `TYPE_DOUBLE` | [L203](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L203) |
| `DECIMAL` | `D` | `TYPE_DOUBLE` | [L204](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L204) |
| `RAD` | `I` | `TYPE_DOUBLE` | [L206](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L206) |
| `DEG` | `D` | `TYPE_INT` | [L207](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L207) |
| `ABS` | `D` | `TYPE_DOUBLE` | [L209](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L209) |
| `EXP` | `D` | `TYPE_DOUBLE` | [L210](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L210) |
| `LOG` | `D` | `TYPE_DOUBLE` | [L211](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L211) |
| `LOG10` | `D` | `TYPE_DOUBLE` | [L212](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L212) |
| `POW` | `DD` | `TYPE_DOUBLE` | [L213](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L213) |
| `SQRT` | `D` | `TYPE_DOUBLE` | [L214](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L214) |
| `FMOD` | `DD` | `TYPE_DOUBLE` | [L215](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L215) |
| `MMOD` | `DD` | `TYPE_DOUBLE` | [L216](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L216) |
| `MODULUS` | `DD` | `TYPE_DOUBLE` | [L217](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L217) |
| `COS` | `I` | `TYPE_DOUBLE` | [L219](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L219) |
| `SIN` | `I` | `TYPE_DOUBLE` | [L220](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L220) |
| `TAN` | `I` | `TYPE_DOUBLE` | [L221](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L221) |
| `ACOS` | `D` | `TYPE_DOUBLE` | [L222](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L222) |
| `ASIN` | `D` | `TYPE_DOUBLE` | [L223](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L223) |
| `ATAN` | `D` | `TYPE_DOUBLE` | [L224](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L224) |
| `ATAN2` | `DD` | `TYPE_DOUBLE` | [L225](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L225) |
| `ISINF` | `D` | `TYPE_INT` | [L227](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L227) |
| `ISNAN` | `D` | `TYPE_INT` | [L228](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L228) |
| `FINITE` | `D` | `TYPE_INT` | [L229](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L229) |
| `INTERSECT` | `DDDDDDDDPP` | `TYPE_INT` | [L231](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L231) |
| `INTERSECT_LINE_CIRCLE` | `DDDDDDDPPPP` | `TYPE_INT` | [L232](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L232) |
| `INTERSECT_CIRCLE` | `DDDDDDPPPP` | `TYPE_INT` | [L233](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L233) |
| `ORTHO` | `DDDDDDPP` | `TYPE_DOUBLE` | [L234](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L234) |
| `PROJECT` | `DDDDDDPPPP` | `TYPE_DOUBLE` | [L235](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L235) |
| `FGET_ANGLE` | `DDDD` | `TYPE_INT` | [L237](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L237) |
| `FGET_DIST` | `DDDD` | `TYPE_DOUBLE` | [L238](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L238) |
| `DISTANCE` | `DDDD` | `TYPE_DOUBLE` | [L239](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L239) |
| `DIST` | `DDDD` | `TYPE_DOUBLE` | [L240](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L240) |
| `NEAR_ANGLE` | `III` | `TYPE_INT` | [L241](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L241) |
| `GET_DISTX` | `ID` | `TYPE_DOUBLE` | [L242](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L242) |
| `GET_DISTY` | `ID` | `TYPE_DOUBLE` | [L243](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L243) |
| `MAG` | `DD` | `TYPE_DOUBLE` | [L245](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L245) |
| `CLAMP` | `DDD` | `TYPE_DOUBLE` | [L247](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L247) |
| `CLAMP` | `III` | `TYPE_INT` | [L248](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L248) |
| `BETWEEN` | `DDD` | `TYPE_INT` | [L249](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L249) |
| `BETWEEN` | `III` | `TYPE_INT` | [L250](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L250) |
| `TOWARDS` | `DDD` | `TYPE_DOUBLE` | [L251](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L251) |
| `TOWARDS` | `III` | `TYPE_INT` | [L252](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L252) |
| `WRAP` | `III` | `TYPE_INT` | [L253](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L253) |
| `LERP` | `DDD` | `TYPE_DOUBLE` | [L254](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L254) |
| `INVLERP` | `DDD` | `TYPE_DOUBLE` | [L255](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L255) |
| `RANGECHK` | `DDD` | `TYPE_INT` | [L256](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L256) |
| `RANGECHK` | `III` | `TYPE_INT` | [L257](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L257) |
| `REMAP` | `DDDDD` | `TYPE_DOUBLE` | [L258](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L258) |
| `REMAP` | `IIIII` | `TYPE_INT` | [L259](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L259) |
| `NORMALIZE` | `DDD` | `TYPE_DOUBLE` | [L260](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L260) |
| `NORMALIZE` | `III` | `TYPE_DOUBLE` | [L261](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L261) |
| `MEM_CALLOC` | `II` | `TYPE_POINTER` | [L264](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L264) |
| `MEM_ALLOC` | `I` | `TYPE_POINTER` | [L265](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L265) |
| `MEM_FREE` | `P` | `TYPE_INT` | [L266](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L266) |
| `MEM_REALLOC` | `PI` | `TYPE_POINTER` | [L267](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L267) |
| `MEM_CMP` | `PPI` | `TYPE_INT` | [L268](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L268) |
| `MEM_SET` | `PII` | `TYPE_INT` | [L269](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L269) |
| `MEM_SETW` | `PII` | `TYPE_INT` | [L270](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L270) |
| `MEM_SETD` | `PII` | `TYPE_INT` | [L271](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L271) |
| `MEM_SETI` | `PII` | `TYPE_INT` | [L272](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L272) |
| `MEM_COPY` | `PPI` | `TYPE_INT` | [L273](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L273) |
| `MEM_MOVE` | `PPI` | `TYPE_INT` | [L274](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L274) |
| `MEM_AVAILABLE` | `` | `TYPE_INT` | [L276](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L276) |
| `MEM_TOTAL` | `` | `TYPE_INT` | [L277](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L277) |
| `CALLOC` | `II` | `TYPE_POINTER` | [L279](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L279) |
| `ALLOC` | `I` | `TYPE_POINTER` | [L280](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L280) |
| `FREE` | `P` | `TYPE_INT` | [L281](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L281) |
| `REALLOC` | `PI` | `TYPE_POINTER` | [L282](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L282) |
| `MEMCMP` | `PPI` | `TYPE_INT` | [L283](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L283) |
| `MEMSET` | `PII` | `TYPE_INT` | [L284](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L284) |
| `MEMSETW` | `PII` | `TYPE_INT` | [L285](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L285) |
| `MEMSETD` | `PII` | `TYPE_INT` | [L286](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L286) |
| `MEMSETI` | `PII` | `TYPE_INT` | [L287](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L287) |
| `MEMCOPY` | `PPI` | `TYPE_INT` | [L288](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L288) |
| `MEMMOVE` | `PPI` | `TYPE_INT` | [L289](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L289) |
| `MEMORY_FREE` | `` | `TYPE_INT` | [L291](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L291) |
| `MEMORY_TOTAL` | `` | `TYPE_INT` | [L292](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L292) |
| `GET_ID` | `I` | `TYPE_INT` | [L295](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L295) |
| `GET_TYPE` | `I` | `TYPE_INT` | [L296](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L296) |
| `GET_STATUS` | `I` | `TYPE_QWORD` | [L297](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L297) |
| `SIGNAL` | `II` | `TYPE_INT` | [L298](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L298) |
| `SIGNAL_ACTION` | `II` | `TYPE_INT` | [L299](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L299) |
| `SIGNAL_ACTION` | `III` | `TYPE_INT` | [L300](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L300) |
| `LET_ME_ALONE` | `` | `TYPE_INT` | [L301](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L301) |
| `EXIT` | `SI` | `TYPE_INT` | [L302](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L302) |
| `EXIT` | `S` | `TYPE_INT` | [L303](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L303) |
| `EXIT` | `` | `TYPE_INT` | [L304](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L304) |
| `EXISTS` | `I` | `TYPE_INT` | [L305](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L305) |
| `PAUSE` | `` | `TYPE_INT` | [L306](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L306) |
| `RESUME` | `` | `TYPE_INT` | [L307](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L307) |
| `RAND_SEED` | `I` | `TYPE_INT` | [L313](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L313) |
| `RAND` | `II` | `TYPE_INT` | [L314](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L314) |
| `REGEX` | `SS` | `TYPE_INT` | [L317](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L317) |
| `REGEX_REPLACE` | `SSS` | `TYPE_STRING` | [L318](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L318) |
| `SPLIT` | `SSPI` | `TYPE_INT` | [L319](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L319) |
| `JOIN` | `SPI` | `TYPE_STRING` | [L320](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L320) |
| `SAY` | `S` | `TYPE_UNDEFINED` | [L322](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L322) |
| `SAY_FAST` | `S` | `TYPE_UNDEFINED` | [L323](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L323) |
| `ISORT` | `PIIIBB` | `TYPE_INT` | [L326](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L326) |
| `KSORT` | `V++V++` | `TYPE_INT` | [L328](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L328) |
| `KSORT` | `V++V++I` | `TYPE_INT` | [L329](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L329) |
| `SORT` | `V++I` | `TYPE_INT` | [L331](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L331) |
| `SORT` | `V++` | `TYPE_INT` | [L332](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L332) |
| `STRLEN` | `S` | `TYPE_INT` | [L335](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L335) |
| `LEN` | `S` | `TYPE_INT` | [L336](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L336) |
| `UCASE` | `S` | `TYPE_STRING` | [L337](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L337) |
| `LCASE` | `S` | `TYPE_STRING` | [L338](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L338) |
| `STRCASECMP` | `SS` | `TYPE_INT` | [L339](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L339) |
| `SUBSTR` | `SII` | `TYPE_STRING` | [L340](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L340) |
| `SUBSTR` | `SI` | `TYPE_STRING` | [L341](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L341) |
| `FIND` | `SS` | `TYPE_INT` | [L342](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L342) |
| `FIND` | `SSI` | `TYPE_INT` | [L343](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L343) |
| `RFIND` | `SS` | `TYPE_INT` | [L344](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L344) |
| `RFIND` | `SSI` | `TYPE_INT` | [L345](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L345) |
| `STRTOK` | `SS` | `TYPE_STRING` | [L346](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L346) |
| `LPAD` | `SI` | `TYPE_STRING` | [L347](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L347) |
| `RPAD` | `SI` | `TYPE_STRING` | [L348](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L348) |
| `ITOA` | `I` | `TYPE_STRING` | [L349](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L349) |
| `FTOA` | `D` | `TYPE_STRING` | [L350](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L350) |
| `ATOI` | `S` | `TYPE_INT` | [L351](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L351) |
| `ATOF` | `S` | `TYPE_DOUBLE` | [L352](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L352) |
| `ASC` | `S` | `TYPE_BYTE` | [L353](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L353) |
| `CHR` | `I` | `TYPE_STRING` | [L354](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L354) |
| `TRIM` | `S` | `TYPE_STRING` | [L355](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L355) |
| `STRREV` | `S` | `TYPE_STRING` | [L356](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L356) |
| `FORMAT` | `I` | `TYPE_STRING` | [L357](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L357) |
| `FORMAT` | `D` | `TYPE_STRING` | [L358](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L358) |
| `FORMAT` | `DI` | `TYPE_STRING` | [L359](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L359) |
| `GETENV` | `S` | `TYPE_STRING` | [L361](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L361) |
| `EXEC` | `ISIP` | `TYPE_INT` | [L362](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L362) |
| `GET_PREF_LANGUAGE` | `` | `TYPE_STRING` | [L363](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L363) |
| `UPTIME` | `` | `TYPE_QWORD` | [L366](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L366) |
| `GET_TIMER` | `` | `TYPE_QWORD` | [L367](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L367) |
| `TIME` | `` | `TYPE_QWORD` | [L368](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L368) |
| `FTIME` | `SI` | `TYPE_STRING` | [L369](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L369) |
| `STRING_NEW_ARRAY` | `` | `TYPE_POINTER` | [L371](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L371) |
| `STRING_NEW_ARRAY` | `I` | `TYPE_POINTER` | [L372](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L372) |
| `STRING_RESIZE_ARRAY` | `PI` | `TYPE_POINTER` | [L373](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L373) |
| `STRING_DEL_ARRAY` | `P` | `TYPE_INT` | [L374](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L374) |
| `STRING_SIZE_ARRAY` | `P` | `TYPE_INT` | [L375](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L375) |
| `STRING_NEWA` | `` | `TYPE_POINTER` | [L377](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L377) |
| `STRING_NEWA` | `I` | `TYPE_POINTER` | [L378](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L378) |
| `STRING_RESIZEA` | `PI` | `TYPE_POINTER` | [L379](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L379) |
| `STRING_DELA` | `P` | `TYPE_INT` | [L380](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L380) |
| `STRING_SIZEA` | `P` | `TYPE_INT` | [L381](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L381) |
| `STRING_DUMP` | `` | `TYPE_INT` | [L383](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L383) |
| `ISALPHA` | `B` | `TYPE_INT` | [L385](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L385) |
| `ISSPACE` | `B` | `TYPE_INT` | [L386](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L386) |
| `ISNUM` | `B` | `TYPE_INT` | [L387](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L387) |
| `ISXNUM` | `B` | `TYPE_INT` | [L388](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L388) |
| `ISALNUM` | `B` | `TYPE_INT` | [L389](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L389) |
| `ISWORDCHAR` | `B` | `TYPE_INT` | [L390](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L390) |
| `ISWORDFIRST` | `B` | `TYPE_INT` | [L391](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L391) |
| `TOUPPER` | `B` | `TYPE_INT` | [L392](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L392) |
| `TOLOWER` | `B` | `TYPE_INT` | [L393](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L393) |
| `LIST_CREATE` | `` | `TYPE_POINTER` | [L396](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L396) |
| `LIST_CREATE` | `I` | `TYPE_POINTER` | [L397](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L397) |
| `LIST_FREE` | `P` | `TYPE_INT` | [L398](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L398) |
| `LIST_EMPTY` | `P` | `TYPE_INT` | [L399](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L399) |
| `LIST_INSERTITEM` | `PP` | `TYPE_INT` | [L400](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L400) |
| `LIST_REMOVEITEM` | `PP` | `TYPE_INT` | [L401](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L401) |
| `LIST_WALK` | `PP` | `TYPE_POINTER` | [L402](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L402) |
| `LIST_SIZE` | `P` | `TYPE_INT` | [L403](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_misc/libmod_misc_exports.h#L403) |

## bennugd2 / libmod_net

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `NET_OPEN` | `IISI` | `TYPE_POINTER` | [L61](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_net/libmod_net_exports.h#L61) |
| `NET_WAIT` | `PIP` | `TYPE_INT` | [L62](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_net/libmod_net_exports.h#L62) |
| `NET_SEND` | `PPI` | `TYPE_INT` | [L63](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_net/libmod_net_exports.h#L63) |
| `NET_RECV` | `PPI` | `TYPE_INT` | [L64](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_net/libmod_net_exports.h#L64) |
| `NET_CLOSE` | `P` | `TYPE_INT` | [L65](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_net/libmod_net_exports.h#L65) |
| `NET_GETAVAILABLEBYTES` | `P` | `TYPE_INT` | [L66](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_net/libmod_net_exports.h#L66) |
| `NET_GETRECEIVEBUFFERSIZE` | `P` | `TYPE_INT` | [L67](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_net/libmod_net_exports.h#L67) |
| `NET_GETREMOTEADDR` | `P` | `TYPE_STRING` | [L68](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_net/libmod_net_exports.h#L68) |
| `NET_IS_NEW_CONNECTION` | `P` | `TYPE_INT` | [L69](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_net/libmod_net_exports.h#L69) |
| `NET_IS_MESSAGE_INCOMING` | `P` | `TYPE_INT` | [L70](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_net/libmod_net_exports.h#L70) |

## bennugd2 / libmod_sound

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `SOUNDSYS_INIT` | `` | `TYPE_INT` | [L95](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L95) |
| `SOUNDSYS_QUIT` | `` | `TYPE_INT` | [L96](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L96) |
| `MUSIC_LOAD` | `S` | `TYPE_INT` | [L97](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L97) |
| `MUSIC_LOAD` | `SP` | `TYPE_INT` | [L98](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L98) |
| `MUSIC_UNLOAD` | `I` | `TYPE_INT` | [L99](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L99) |
| `MUSIC_UNLOAD` | `P` | `TYPE_INT` | [L100](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L100) |
| `MUSIC_PLAY` | `II` | `TYPE_INT` | [L101](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L101) |
| `MUSIC_PLAY` | `I` | `TYPE_INT` | [L102](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L102) |
| `MUSIC_STOP` | `` | `TYPE_INT` | [L103](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L103) |
| `MUSIC_PAUSE` | `` | `TYPE_INT` | [L104](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L104) |
| `MUSIC_RESUME` | `` | `TYPE_INT` | [L105](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L105) |
| `MUSIC_REWIND` | `` | `TYPE_INT` | [L106](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L106) |
| `MUSIC_FADE_IN` | `III` | `TYPE_INT` | [L107](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L107) |
| `MUSIC_FADE_IN` | `IIID` | `TYPE_INT` | [L108](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L108) |
| `MUSIC_FADE_OFF` | `I` | `TYPE_INT` | [L109](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L109) |
| `MUSIC_IS_PLAYING` | `` | `TYPE_INT` | [L110](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L110) |
| `MUSIC_IS_PAUSED` | `` | `TYPE_INT` | [L111](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L111) |
| `MUSIC_IS_FADING` | `` | `TYPE_INT` | [L112](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L112) |
| `MUSIC_SET_VOLUME` | `I` | `TYPE_INT` | [L113](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L113) |
| `MUSIC_GET_VOLUME` | `I` | `TYPE_INT` | [L114](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L114) |
| `MUSIC_SET_PLAYBACK_POSITION` | `D` | `TYPE_INT` | [L115](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L115) |
| `MUSIC_SET_TIME_POSITION` | `D` | `TYPE_INT` | [L116](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L116) |
| `MUSIC_GET_PLAYBACK_POSITION` | `I` | `TYPE_DOUBLE` | [L117](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L117) |
| `MUSIC_GET_TIME_POSITION` | `I` | `TYPE_DOUBLE` | [L118](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L118) |
| `MUSIC_GET_DURATION` | `I` | `TYPE_DOUBLE` | [L119](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L119) |
| `MUSIC_GET_DURATION` | `` | `TYPE_DOUBLE` | [L120](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L120) |
| `MUSIC_GET_LOOP_START` | `I` | `TYPE_DOUBLE` | [L121](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L121) |
| `MUSIC_GET_LOOP_START` | `` | `TYPE_DOUBLE` | [L122](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L122) |
| `MUSIC_GET_LOOP_END` | `I` | `TYPE_DOUBLE` | [L123](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L123) |
| `MUSIC_GET_LOOP_END` | `` | `TYPE_DOUBLE` | [L124](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L124) |
| `MUSIC_GET_LOOP_LEN` | `I` | `TYPE_DOUBLE` | [L125](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L125) |
| `MUSIC_GET_LOOP_LEN` | `` | `TYPE_DOUBLE` | [L126](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L126) |
| `SOUND_LOAD` | `S` | `TYPE_INT` | [L127](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L127) |
| `SOUND_LOAD` | `SP` | `TYPE_INT` | [L128](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L128) |
| `SOUND_UNLOAD` | `I` | `TYPE_INT` | [L129](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L129) |
| `SOUND_UNLOAD` | `P` | `TYPE_INT` | [L130](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L130) |
| `SOUND_PLAY` | `II` | `TYPE_INT` | [L131](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L131) |
| `SOUND_PLAY` | `I` | `TYPE_INT` | [L132](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L132) |
| `SOUND_PLAY` | `III` | `TYPE_INT` | [L133](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L133) |
| `SOUND_PLAY` | `IIII` | `TYPE_INT` | [L134](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L134) |
| `SOUND_STOP` | `I` | `TYPE_INT` | [L135](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L135) |
| `SOUND_PAUSE` | `I` | `TYPE_INT` | [L136](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L136) |
| `SOUND_RESUME` | `I` | `TYPE_INT` | [L137](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L137) |
| `SOUND_FADE_IN` | `IIII` | `TYPE_INT` | [L138](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L138) |
| `SOUND_FADE_IN` | `IIIII` | `TYPE_INT` | [L139](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L139) |
| `SOUND_FADE_OFF` | `II` | `TYPE_INT` | [L140](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L140) |
| `SOUND_IS_PLAYING` | `I` | `TYPE_INT` | [L141](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L141) |
| `SOUND_IS_PAUSED` | `I` | `TYPE_INT` | [L142](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L142) |
| `SOUND_IS_FADING` | `I` | `TYPE_INT` | [L143](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L143) |
| `SOUND_SET_VOLUME` | `II` | `TYPE_INT` | [L144](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L144) |
| `SOUND_SET_VOLUME` | `I` | `TYPE_INT` | [L145](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L145) |
| `SOUND_SET_LOCATION` | `III` | `TYPE_INT` | [L146](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L146) |
| `SOUND_SET_SPATIAL_POSITION` | `III` | `TYPE_INT` | [L147](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L147) |
| `SET_POSITION` | `III` | `TYPE_INT` | [L148](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L148) |
| `CHANNEL_SET_EXPIRE` | `II` | `TYPE_INT` | [L149](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L149) |
| `CHANNEL_SET_EXPIRE` | `I` | `TYPE_INT` | [L150](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L150) |
| `CHANNEL_SET_VOLUME` | `II` | `TYPE_INT` | [L151](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L151) |
| `CHANNEL_SET_VOLUME` | `I` | `TYPE_INT` | [L152](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L152) |
| `SET_MASTER_VOLUME` | `I` | `TYPE_INT` | [L153](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L153) |
| `CHANNEL_SET_PANNING` | `III` | `TYPE_INT` | [L154](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L154) |
| `SET_PANNING` | `III` | `TYPE_INT` | [L155](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L155) |
| `CHANNEL_SET_DISTANCE` | `II` | `TYPE_INT` | [L156](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L156) |
| `SET_DISTANCE` | `II` | `TYPE_INT` | [L157](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L157) |
| `RESERVE_CHANNELS` | `I` | `TYPE_INT` | [L158](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L158) |
| `REVERSE_STEREO` | `II` | `TYPE_INT` | [L159](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L159) |
| `GET_NUM_MUSIC_DECODERS` | `` | `TYPE_INT` | [L160](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L160) |
| `GET_MUSIC_DECODER` | `I` | `TYPE_STRING` | [L161](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L161) |
| `HAS_MUSIC_DECODER` | `S` | `TYPE_INT` | [L162](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L162) |
| `MUSIC_GET_TYPE` | `I` | `TYPE_INT` | [L163](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L163) |
| `MUSIC_GET_TITLE` | `I` | `TYPE_STRING` | [L164](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L164) |
| `MUSIC_GET_TITLE_TAG` | `I` | `TYPE_STRING` | [L165](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L165) |
| `MUSIC_GET_ARTIST_TAG` | `I` | `TYPE_STRING` | [L166](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L166) |
| `MUSIC_GET_ALBUM_TAG` | `I` | `TYPE_STRING` | [L167](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L167) |
| `MUSIC_GET_COPYRIGHT_TAG` | `I` | `TYPE_STRING` | [L168](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L168) |
| `MUSIC_GET_TYPE` | `` | `TYPE_INT` | [L169](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L169) |
| `MUSIC_GET_TITLE` | `` | `TYPE_STRING` | [L170](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L170) |
| `MUSIC_GET_TITLE_TAG` | `` | `TYPE_STRING` | [L171](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L171) |
| `MUSIC_GET_ARTIST_TAG` | `` | `TYPE_STRING` | [L172](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L172) |
| `MUSIC_GET_ALBUM_TAG` | `` | `TYPE_STRING` | [L173](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L173) |
| `MUSIC_GET_COPYRIGHT_TAG` | `` | `TYPE_STRING` | [L174](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L174) |
| `SOUND_GROUP_CHANNEL` | `II` | `TYPE_INT` | [L175](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L175) |
| `SOUND_GROUP_CHANNELS` | `III` | `TYPE_INT` | [L176](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L176) |
| `SOUND_GROUP_AVAILABLE` | `I` | `TYPE_INT` | [L177](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L177) |
| `SOUND_GROUP_COUNT` | `I` | `TYPE_INT` | [L178](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L178) |
| `SOUND_GROUP_OLDEST` | `I` | `TYPE_INT` | [L179](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L179) |
| `SOUND_GROUP_NEWER` | `I` | `TYPE_INT` | [L180](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L180) |
| `SOUND_GROUP_HALT` | `I` | `TYPE_INT` | [L181](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L181) |
| `SOUND_GROUP_FADE_OUT` | `II` | `TYPE_INT` | [L182](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_sound/libmod_sound_exports.h#L182) |

## bennugd1 / mod_blendop

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `BLENDOP_NEW` | `` | `TYPE_INT` | [L41](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_blendop/mod_blendop_exports.h#L41) |
| `BLENDOP_IDENTITY` | `I` | `TYPE_INT` | [L42](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_blendop/mod_blendop_exports.h#L42) |
| `BLENDOP_TINT` | `IFIII` | `TYPE_INT` | [L43](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_blendop/mod_blendop_exports.h#L43) |
| `BLENDOP_TRANSLUCENCY` | `IF` | `TYPE_INT` | [L44](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_blendop/mod_blendop_exports.h#L44) |
| `BLENDOP_INTENSITY` | `IF` | `TYPE_INT` | [L45](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_blendop/mod_blendop_exports.h#L45) |
| `BLENDOP_SWAP` | `I` | `TYPE_INT` | [L46](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_blendop/mod_blendop_exports.h#L46) |
| `BLENDOP_ASSIGN` | `III` | `TYPE_INT` | [L47](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_blendop/mod_blendop_exports.h#L47) |
| `BLENDOP_APPLY` | `III` | `TYPE_INT` | [L48](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_blendop/mod_blendop_exports.h#L48) |
| `BLENDOP_FREE` | `I` | `TYPE_INT` | [L49](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_blendop/mod_blendop_exports.h#L49) |
| `BLENDOP_GRAYSCALE` | `II` | `TYPE_INT` | [L50](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_blendop/mod_blendop_exports.h#L50) |

## bennugd1 / mod_cd

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `CD_DRIVES` | `` | `TYPE_INT` | [L82](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_cd/mod_cd_exports.h#L82) |
| `CD_STATUS` | `I` | `TYPE_INT` | [L83](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_cd/mod_cd_exports.h#L83) |
| `CD_NAME` | `I` | `TYPE_STRING` | [L84](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_cd/mod_cd_exports.h#L84) |
| `CD_GETINFO` | `I` | `TYPE_INT` | [L85](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_cd/mod_cd_exports.h#L85) |
| `CD_PLAY` | `II` | `TYPE_INT` | [L86](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_cd/mod_cd_exports.h#L86) |
| `CD_PLAY` | `III` | `TYPE_INT` | [L87](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_cd/mod_cd_exports.h#L87) |
| `CD_STOP` | `I` | `TYPE_INT` | [L88](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_cd/mod_cd_exports.h#L88) |
| `CD_PAUSE` | `I` | `TYPE_INT` | [L89](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_cd/mod_cd_exports.h#L89) |
| `CD_RESUME` | `I` | `TYPE_INT` | [L90](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_cd/mod_cd_exports.h#L90) |
| `CD_EJECT` | `I` | `TYPE_INT` | [L91](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_cd/mod_cd_exports.h#L91) |

## bennugd1 / mod_crypt

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `CRYPT_NEW` | `IP` | `TYPE_POINTER` | [L62](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_crypt/mod_crypt_exports.h#L62) |
| `CRYPT_DEL` | `P` | `TYPE_INT` | [L63](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_crypt/mod_crypt_exports.h#L63) |
| `CRYPT_ENCRYPT` | `PPPI` | `TYPE_INT` | [L64](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_crypt/mod_crypt_exports.h#L64) |
| `CRYPT_DECRYPT` | `PPPI` | `TYPE_INT` | [L65](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_crypt/mod_crypt_exports.h#L65) |
| `CRYPT_ENCRYPT` | `IPPPI` | `TYPE_INT` | [L66](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_crypt/mod_crypt_exports.h#L66) |
| `CRYPT_DECRYPT` | `IPPPI` | `TYPE_INT` | [L67](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_crypt/mod_crypt_exports.h#L67) |

## bennugd1 / mod_debug

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `TRACE` | `I` | `TYPE_INT` | [L51](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_debug/mod_debug_exports.h#L51) |

## bennugd1 / mod_dir

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `CD` | `` | `TYPE_STRING` | [L64](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_dir/mod_dir_exports.h#L64) |
| `CD` | `S` | `TYPE_INT` | [L65](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_dir/mod_dir_exports.h#L65) |
| `CHDIR` | `S` | `TYPE_INT` | [L66](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_dir/mod_dir_exports.h#L66) |
| `MKDIR` | `S` | `TYPE_INT` | [L67](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_dir/mod_dir_exports.h#L67) |
| `RMDIR` | `S` | `TYPE_INT` | [L68](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_dir/mod_dir_exports.h#L68) |
| `GLOB` | `S` | `TYPE_STRING` | [L69](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_dir/mod_dir_exports.h#L69) |
| `RM` | `S` | `TYPE_INT` | [L70](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_dir/mod_dir_exports.h#L70) |
| `DIROPEN` | `S` | `TYPE_INT` | [L72](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_dir/mod_dir_exports.h#L72) |
| `DIRCLOSE` | `I` | `TYPE_INT` | [L73](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_dir/mod_dir_exports.h#L73) |
| `DIRREAD` | `I` | `TYPE_STRING` | [L74](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_dir/mod_dir_exports.h#L74) |

## bennugd1 / mod_draw

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `DRAWING_MAP` | `II` | `TYPE_INT` | [L42](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_draw/mod_draw_exports.h#L42) |
| `DRAWING_COLOR` | `I` | `TYPE_INT` | [L43](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_draw/mod_draw_exports.h#L43) |
| `DRAW_LINE` | `IIII` | `TYPE_INT` | [L44](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_draw/mod_draw_exports.h#L44) |
| `DRAW_RECT` | `IIII` | `TYPE_INT` | [L45](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_draw/mod_draw_exports.h#L45) |
| `DRAW_BOX` | `IIII` | `TYPE_INT` | [L46](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_draw/mod_draw_exports.h#L46) |
| `DRAW_CIRCLE` | `III` | `TYPE_INT` | [L47](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_draw/mod_draw_exports.h#L47) |
| `DRAW_FCIRCLE` | `III` | `TYPE_INT` | [L48](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_draw/mod_draw_exports.h#L48) |
| `DRAW_CURVE` | `IIIIIIIII` | `TYPE_INT` | [L49](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_draw/mod_draw_exports.h#L49) |
| `DRAWING_Z` | `I` | `TYPE_INT` | [L50](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_draw/mod_draw_exports.h#L50) |
| `DELETE_DRAW` | `I` | `TYPE_INT` | [L51](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_draw/mod_draw_exports.h#L51) |
| `MOVE_DRAW` | `III` | `TYPE_INT` | [L52](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_draw/mod_draw_exports.h#L52) |
| `DRAWING_ALPHA` | `I` | `TYPE_INT` | [L53](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_draw/mod_draw_exports.h#L53) |
| `DRAWING_STIPPLE` | `I` | `TYPE_INT` | [L54](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_draw/mod_draw_exports.h#L54) |
| `PUT_PIXEL` | `III` | `TYPE_INT` | [L55](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_draw/mod_draw_exports.h#L55) |
| `GET_PIXEL` | `II` | `TYPE_INT` | [L56](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_draw/mod_draw_exports.h#L56) |
| `MAP_GET_PIXEL` | `IIII` | `TYPE_INT` | [L57](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_draw/mod_draw_exports.h#L57) |
| `MAP_PUT_PIXEL` | `IIIII` | `TYPE_INT` | [L58](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_draw/mod_draw_exports.h#L58) |

## bennugd1 / mod_effects

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `GRAYSCALE` | `IIB` | `TYPE_INT` | [L70](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_effects/mod_effects_exports.h#L70) |
| `RGBSCALE` | `IIFFF` | `TYPE_INT` | [L71](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_effects/mod_effects_exports.h#L71) |
| `BLUR` | `IIB` | `TYPE_INT` | [L72](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_effects/mod_effects_exports.h#L72) |
| `FILTER` | `IIP` | `TYPE_INT` | [L73](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_effects/mod_effects_exports.h#L73) |

## bennugd1 / mod_file

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `SAVE` | `SV++` | `TYPE_INT` | [L66](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_file/mod_file_exports.h#L66) |
| `LOAD` | `SV++` | `TYPE_INT` | [L67](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_file/mod_file_exports.h#L67) |
| `FOPEN` | `SI` | `TYPE_INT` | [L68](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_file/mod_file_exports.h#L68) |
| `FCLOSE` | `I` | `TYPE_INT` | [L69](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_file/mod_file_exports.h#L69) |
| `FREAD` | `IV++` | `TYPE_INT` | [L70](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_file/mod_file_exports.h#L70) |
| `FREAD` | `PII` | `TYPE_INT` | [L71](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_file/mod_file_exports.h#L71) |
| `FWRITE` | `IV++` | `TYPE_INT` | [L72](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_file/mod_file_exports.h#L72) |
| `FWRITE` | `PII` | `TYPE_INT` | [L73](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_file/mod_file_exports.h#L73) |
| `FSEEK` | `III` | `TYPE_INT` | [L74](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_file/mod_file_exports.h#L74) |
| `FREWIND` | `I` | `TYPE_UNDEFINED` | [L75](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_file/mod_file_exports.h#L75) |
| `FTELL` | `I` | `TYPE_INT` | [L76](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_file/mod_file_exports.h#L76) |
| `FFLUSH` | `I` | `TYPE_INT` | [L77](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_file/mod_file_exports.h#L77) |
| `FLUSH` | `I` | `TYPE_INT` | [L78](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_file/mod_file_exports.h#L78) |
| `FLENGTH` | `I` | `TYPE_INT` | [L79](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_file/mod_file_exports.h#L79) |
| `FPUTS` | `IS` | `TYPE_INT` | [L80](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_file/mod_file_exports.h#L80) |
| `FGETS` | `I` | `TYPE_STRING` | [L81](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_file/mod_file_exports.h#L81) |
| `FEOF` | `I` | `TYPE_INT` | [L82](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_file/mod_file_exports.h#L82) |
| `FILE` | `S` | `TYPE_STRING` | [L83](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_file/mod_file_exports.h#L83) |
| `FEXISTS` | `S` | `TYPE_INT` | [L84](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_file/mod_file_exports.h#L84) |
| `FILE_EXISTS` | `S` | `TYPE_INT` | [L85](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_file/mod_file_exports.h#L85) |
| `FREMOVE` | `S` | `TYPE_INT` | [L86](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_file/mod_file_exports.h#L86) |
| `FMOVE` | `SS` | `TYPE_INT` | [L87](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_file/mod_file_exports.h#L87) |

## bennugd1 / mod_flic

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `START_FLI` | `SII` | `TYPE_INT` | [L42](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_flic/mod_flic_exports.h#L42) |
| `END_FLI` | `` | `TYPE_INT` | [L43](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_flic/mod_flic_exports.h#L43) |
| `FRAME_FLI` | `` | `TYPE_INT` | [L44](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_flic/mod_flic_exports.h#L44) |
| `RESET_FLI` | `` | `TYPE_INT` | [L45](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_flic/mod_flic_exports.h#L45) |
| `FLI_START` | `SII` | `TYPE_INT` | [L47](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_flic/mod_flic_exports.h#L47) |
| `FLI_START` | `SIIIIII` | `TYPE_INT` | [L48](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_flic/mod_flic_exports.h#L48) |
| `FLI_END` | `I` | `TYPE_INT` | [L49](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_flic/mod_flic_exports.h#L49) |
| `FLI_FRAME` | `I` | `TYPE_INT` | [L50](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_flic/mod_flic_exports.h#L50) |
| `FLI_RESET` | `I` | `TYPE_INT` | [L51](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_flic/mod_flic_exports.h#L51) |
| `FLI_PARAMS` | `IIIIIII` | `TYPE_INT` | [L53](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_flic/mod_flic_exports.h#L53) |
| `FLI_MOVE` | `III` | `TYPE_INT` | [L54](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_flic/mod_flic_exports.h#L54) |
| `FLI_ANGLE` | `II` | `TYPE_INT` | [L55](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_flic/mod_flic_exports.h#L55) |
| `FLI_SIZE` | `II` | `TYPE_INT` | [L56](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_flic/mod_flic_exports.h#L56) |
| `FLI_FLAGS` | `II` | `TYPE_INT` | [L57](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_flic/mod_flic_exports.h#L57) |
| `FLI_Z` | `II` | `TYPE_INT` | [L58](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_flic/mod_flic_exports.h#L58) |
| `FLI_GETINFO` | `IPPPPPPPPP` | `TYPE_INT` | [L59](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_flic/mod_flic_exports.h#L59) |

## bennugd1 / mod_grproc

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `ADVANCE` | `I` | `TYPE_INT` | [L56](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_grproc/mod_grproc_exports.h#L56) |
| `XADVANCE` | `II` | `TYPE_INT` | [L57](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_grproc/mod_grproc_exports.h#L57) |
| `GET_ANGLE` | `I` | `TYPE_INT` | [L59](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_grproc/mod_grproc_exports.h#L59) |
| `GET_DIST` | `I` | `TYPE_INT` | [L60](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_grproc/mod_grproc_exports.h#L60) |
| `COLLISION` | `I` | `TYPE_INT` | [L61](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_grproc/mod_grproc_exports.h#L61) |
| `COLLISION_BOX` | `I` | `TYPE_INT` | [L62](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_grproc/mod_grproc_exports.h#L62) |
| `COLLISION_CIRCLE` | `I` | `TYPE_INT` | [L63](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_grproc/mod_grproc_exports.h#L63) |
| `GET_REAL_POINT` | `IPP` | `TYPE_INT` | [L65](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_grproc/mod_grproc_exports.h#L65) |

## bennugd1 / mod_joy

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `JOY_AXES` | `` | `TYPE_INT` | [L40](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_joy/mod_joy_exports.h#L40) |
| `JOY_AXES` | `I` | `TYPE_INT` | [L41](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_joy/mod_joy_exports.h#L41) |
| `JOY_NUMAXES` | `` | `TYPE_INT` | [L42](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_joy/mod_joy_exports.h#L42) |
| `JOY_NUMAXES` | `I` | `TYPE_INT` | [L43](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_joy/mod_joy_exports.h#L43) |
| `JOY_GETAXIS` | `I` | `TYPE_INT` | [L44](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_joy/mod_joy_exports.h#L44) |
| `JOY_GETAXIS` | `II` | `TYPE_INT` | [L45](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_joy/mod_joy_exports.h#L45) |
| `JOY_BUTTONS` | `` | `TYPE_INT` | [L47](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_joy/mod_joy_exports.h#L47) |
| `JOY_BUTTONS` | `I` | `TYPE_INT` | [L48](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_joy/mod_joy_exports.h#L48) |
| `JOY_NAME` | `I` | `TYPE_STRING` | [L50](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_joy/mod_joy_exports.h#L50) |
| `JOY_NUMBUTTONS` | `` | `TYPE_INT` | [L52](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_joy/mod_joy_exports.h#L52) |
| `JOY_NUMBUTTONS` | `I` | `TYPE_INT` | [L53](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_joy/mod_joy_exports.h#L53) |
| `JOY_NUMBER` | `` | `TYPE_INT` | [L55](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_joy/mod_joy_exports.h#L55) |
| `JOY_NUMJOYSTICKS` | `` | `TYPE_INT` | [L56](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_joy/mod_joy_exports.h#L56) |
| `JOY_SELECT` | `I` | `TYPE_INT` | [L58](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_joy/mod_joy_exports.h#L58) |
| `JOY_GETBUTTON` | `I` | `TYPE_INT` | [L60](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_joy/mod_joy_exports.h#L60) |
| `JOY_GETBUTTON` | `II` | `TYPE_INT` | [L61](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_joy/mod_joy_exports.h#L61) |
| `JOY_GETPOSITION` | `I` | `TYPE_INT` | [L62](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_joy/mod_joy_exports.h#L62) |
| `JOY_GETPOSITION` | `II` | `TYPE_INT` | [L63](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_joy/mod_joy_exports.h#L63) |
| `JOY_NUMHATS` | `` | `TYPE_INT` | [L65](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_joy/mod_joy_exports.h#L65) |
| `JOY_NUMHATS` | `I` | `TYPE_INT` | [L66](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_joy/mod_joy_exports.h#L66) |
| `JOY_NUMBALLS` | `` | `TYPE_INT` | [L67](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_joy/mod_joy_exports.h#L67) |
| `JOY_NUMBALLS` | `I` | `TYPE_INT` | [L68](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_joy/mod_joy_exports.h#L68) |
| `JOY_GETHAT` | `I` | `TYPE_INT` | [L69](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_joy/mod_joy_exports.h#L69) |
| `JOY_GETHAT` | `II` | `TYPE_INT` | [L70](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_joy/mod_joy_exports.h#L70) |
| `JOY_GETBALL` | `IPP` | `TYPE_INT` | [L71](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_joy/mod_joy_exports.h#L71) |
| `JOY_GETBALL` | `IIPP` | `TYPE_INT` | [L72](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_joy/mod_joy_exports.h#L72) |
| `JOY_GETACCEL` | `PPP` | `TYPE_INT` | [L74](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_joy/mod_joy_exports.h#L74) |
| `JOY_GETACCEL` | `IPPP` | `TYPE_INT` | [L75](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_joy/mod_joy_exports.h#L75) |
| `NUMBER_JOY` | `` | `TYPE_INT` | [L79](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_joy/mod_joy_exports.h#L79) |
| `SELECT_JOY` | `I` | `TYPE_INT` | [L80](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_joy/mod_joy_exports.h#L80) |
| `GET_JOY_BUTTON` | `I` | `TYPE_INT` | [L81](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_joy/mod_joy_exports.h#L81) |
| `GET_JOY_BUTTON` | `II` | `TYPE_INT` | [L82](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_joy/mod_joy_exports.h#L82) |
| `GET_JOY_POSITION` | `I` | `TYPE_INT` | [L83](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_joy/mod_joy_exports.h#L83) |
| `GET_JOY_POSITION` | `II` | `TYPE_INT` | [L84](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_joy/mod_joy_exports.h#L84) |

## bennugd1 / mod_key

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `KEY` | `I` | `TYPE_INT` | [L40](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_key/mod_key_exports.h#L40) |

## bennugd1 / mod_m7

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `MODE7_START` | `IIIIIIII` | `TYPE_INT` | [L83](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_m7/mod_m7_exports.h#L83) |
| `MODE7_START` | `IIIIII` | `TYPE_INT` | [L84](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_m7/mod_m7_exports.h#L84) |
| `MODE7_STOP` | `I` | `TYPE_INT` | [L85](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_m7/mod_m7_exports.h#L85) |
| `START_MODE7` | `IIIIIIII` | `TYPE_INT` | [L87](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_m7/mod_m7_exports.h#L87) |
| `START_MODE7` | `IIIIII` | `TYPE_INT` | [L88](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_m7/mod_m7_exports.h#L88) |
| `STOP_MODE7` | `I` | `TYPE_INT` | [L89](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_m7/mod_m7_exports.h#L89) |

## bennugd1 / mod_map

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `MAP_BLOCK_COPY` | `IIIIIIIIII` | `TYPE_INT` | [L77](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L77) |
| `MAP_PUT` | `IIIII` | `TYPE_INT` | [L78](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L78) |
| `MAP_XPUT` | `IIIIIIII` | `TYPE_INT` | [L79](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L79) |
| `MAP_NEW` | `III` | `TYPE_INT` | [L80](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L80) |
| `MAP_NEW` | `IIII` | `TYPE_INT` | [L81](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L81) |
| `MAP_CLEAR` | `III` | `TYPE_INT` | [L82](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L82) |
| `MAP_CLONE` | `II` | `TYPE_INT` | [L83](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L83) |
| `MAP_NAME` | `II` | `TYPE_STRING` | [L84](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L84) |
| `MAP_SET_NAME` | `IIS` | `TYPE_INT` | [L85](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L85) |
| `MAP_EXISTS` | `II` | `TYPE_INT` | [L86](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L86) |
| `MAP_XPUTNP` | `IIIIIIIIII` | `TYPE_INT` | [L87](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L87) |
| `MAP_DEL` | `II` | `TYPE_INT` | [L88](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L88) |
| `MAP_UNLOAD` | `II` | `TYPE_INT` | [L89](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L89) |
| `MAP_LOAD` | `S` | `TYPE_INT` | [L90](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L90) |
| `MAP_LOAD` | `SP` | `TYPE_INT` | [L91](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L91) |
| `MAP_SAVE` | `IIS` | `TYPE_INT` | [L92](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L92) |
| `MAP_BUFFER` | `II` | `TYPE_POINTER` | [L93](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L93) |
| `FPG_ADD` | `IIII` | `TYPE_INT` | [L96](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L96) |
| `FPG_NEW` | `` | `TYPE_INT` | [L97](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L97) |
| `FPG_EXISTS` | `I` | `TYPE_INT` | [L98](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L98) |
| `FPG_LOAD` | `S` | `TYPE_INT` | [L99](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L99) |
| `FPG_LOAD` | `SP` | `TYPE_INT` | [L100](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L100) |
| `FPG_SAVE` | `IS` | `TYPE_INT` | [L101](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L101) |
| `FPG_DEL` | `I` | `TYPE_INT` | [L102](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L102) |
| `FPG_UNLOAD` | `I` | `TYPE_INT` | [L103](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L103) |
| `PAL_NEW` | `` | `TYPE_INT` | [L106](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L106) |
| `PAL_DEL` | `I` | `TYPE_INT` | [L107](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L107) |
| `PAL_UNLOAD` | `I` | `TYPE_INT` | [L108](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L108) |
| `PAL_CLONE` | `I` | `TYPE_INT` | [L109](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L109) |
| `PAL_REFRESH` | `` | `TYPE_INT` | [L110](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L110) |
| `PAL_REFRESH` | `I` | `TYPE_INT` | [L111](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L111) |
| `PAL_MAP_GETID` | `II` | `TYPE_INT` | [L112](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L112) |
| `PAL_MAP_ASSIGN` | `III` | `TYPE_INT` | [L113](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L113) |
| `PAL_MAP_REMOVE` | `II` | `TYPE_INT` | [L114](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L114) |
| `PAL_GET` | `IIP` | `TYPE_INT` | [L115](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L115) |
| `PAL_GET` | `IIIP` | `TYPE_INT` | [L116](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L116) |
| `PAL_SYS_SET` | `I` | `TYPE_INT` | [L117](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L117) |
| `PAL_SYS_SET` | `P` | `TYPE_INT` | [L118](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L118) |
| `PAL_SET` | `IIP` | `TYPE_INT` | [L119](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L119) |
| `PAL_SET` | `IIIP` | `TYPE_INT` | [L120](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L120) |
| `PAL_SAVE` | `S` | `TYPE_INT` | [L121](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L121) |
| `PAL_SAVE` | `SI` | `TYPE_INT` | [L122](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L122) |
| `PAL_LOAD` | `S` | `TYPE_INT` | [L123](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L123) |
| `PAL_LOAD` | `SP` | `TYPE_INT` | [L124](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L124) |
| `COLORS_SET` | `IIP` | `TYPE_INT` | [L126](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L126) |
| `COLORS_SET` | `IIIP` | `TYPE_INT` | [L127](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L127) |
| `COLORS_GET` | `IIP` | `TYPE_INT` | [L128](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L128) |
| `COLORS_GET` | `IIIP` | `TYPE_INT` | [L129](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L129) |
| `PALETTE_ROLL` | `III` | `TYPE_INT` | [L131](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L131) |
| `PALETTE_CONVERT` | `IIP` | `TYPE_INT` | [L132](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L132) |
| `COLOR_FIND` | `BBB` | `TYPE_INT` | [L134](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L134) |
| `COLOR_FIND` | `IBBB` | `TYPE_INT` | [L135](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L135) |
| `COLOR_FIND` | `IIBBB` | `TYPE_INT` | [L136](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L136) |
| `RGB` | `IIBBB` | `TYPE_INT` | [L138](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L138) |
| `RGBA` | `IIBBBB` | `TYPE_INT` | [L139](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L139) |
| `RGB` | `BBBI` | `TYPE_INT` | [L140](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L140) |
| `RGBA` | `BBBBI` | `TYPE_INT` | [L141](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L141) |
| `RGB` | `BBB` | `TYPE_INT` | [L142](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L142) |
| `RGBA` | `BBBB` | `TYPE_INT` | [L143](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L143) |
| `RGB_GET` | `IIIPPP` | `TYPE_INT` | [L145](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L145) |
| `RGBA_GET` | `IIIPPPP` | `TYPE_INT` | [L146](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L146) |
| `RGB_GET` | `IPPPI` | `TYPE_INT` | [L147](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L147) |
| `RGBA_GET` | `IPPPPI` | `TYPE_INT` | [L148](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L148) |
| `RGB_GET` | `IPPP` | `TYPE_INT` | [L149](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L149) |
| `RGBA_GET` | `IPPPP` | `TYPE_INT` | [L150](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L150) |
| `FADE` | `IIII` | `TYPE_INT` | [L152](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L152) |
| `FADE_ON` | `` | `TYPE_INT` | [L153](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L153) |
| `FADE_OFF` | `` | `TYPE_INT` | [L154](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L154) |
| `MAP_INFO_SET` | `IIII` | `TYPE_INT` | [L157](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L157) |
| `MAP_INFO_GET` | `III` | `TYPE_INT` | [L158](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L158) |
| `MAP_INFO` | `III` | `TYPE_INT` | [L159](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L159) |
| `GRAPHIC_SET` | `IIII` | `TYPE_INT` | [L161](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L161) |
| `GRAPHIC_INFO` | `III` | `TYPE_INT` | [L162](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L162) |
| `POINT_GET` | `IIIPP` | `TYPE_INT` | [L165](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L165) |
| `POINT_SET` | `IIIII` | `TYPE_INT` | [L166](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L166) |
| `CENTER_SET` | `IIII` | `TYPE_INT` | [L168](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L168) |
| `FNT_LOAD` | `S` | `TYPE_INT` | [L171](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L171) |
| `FNT_LOAD` | `SP` | `TYPE_INT` | [L172](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L172) |
| `FNT_UNLOAD` | `I` | `TYPE_INT` | [L173](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L173) |
| `FNT_SAVE` | `IS` | `TYPE_INT` | [L174](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L174) |
| `FNT_NEW` | `I` | `TYPE_INT` | [L175](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L175) |
| `FNT_NEW` | `II` | `TYPE_INT` | [L176](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L176) |
| `FNT_NEW` | `IIIIIIII` | `TYPE_INT` | [L177](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L177) |
| `BDF_LOAD` | `S` | `TYPE_INT` | [L179](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L179) |
| `BDF_LOAD` | `SP` | `TYPE_INT` | [L180](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L180) |
| `GLYPH_GET` | `II` | `TYPE_INT` | [L182](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L182) |
| `GLYPH_SET` | `IIII` | `TYPE_INT` | [L183](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L183) |
| `PNG_LOAD` | `S` | `TYPE_INT` | [L186](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L186) |
| `PNG_LOAD` | `SP` | `TYPE_INT` | [L187](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L187) |
| `PCX_LOAD` | `S` | `TYPE_INT` | [L188](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L188) |
| `PCX_LOAD` | `SP` | `TYPE_INT` | [L189](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L189) |
| `PNG_SAVE` | `IIS` | `TYPE_INT` | [L192](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L192) |
| `NEW_MAP` | `III` | `TYPE_INT` | [L197](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L197) |
| `LOAD_MAP` | `S` | `TYPE_INT` | [L198](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L198) |
| `LOAD_MAP` | `SP` | `TYPE_INT` | [L199](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L199) |
| `UNLOAD_MAP` | `II` | `TYPE_INT` | [L200](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L200) |
| `SAVE_MAP` | `IIS` | `TYPE_INT` | [L201](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L201) |
| `NEW_PAL` | `` | `TYPE_INT` | [L204](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L204) |
| `LOAD_PAL` | `S` | `TYPE_INT` | [L205](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L205) |
| `LOAD_PAL` | `SP` | `TYPE_INT` | [L206](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L206) |
| `UNLOAD_PAL` | `I` | `TYPE_INT` | [L207](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L207) |
| `SAVE_PAL` | `S` | `TYPE_INT` | [L208](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L208) |
| `SAVE_PAL` | `SI` | `TYPE_INT` | [L209](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L209) |
| `SET_COLORS` | `IIP` | `TYPE_INT` | [L211](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L211) |
| `SET_COLORS` | `IIIP` | `TYPE_INT` | [L212](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L212) |
| `GET_COLORS` | `IIP` | `TYPE_INT` | [L213](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L213) |
| `GET_COLORS` | `IIIP` | `TYPE_INT` | [L214](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L214) |
| `ROLL_PALETTE` | `III` | `TYPE_INT` | [L216](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L216) |
| `CONVERT_PALETTE` | `IIP` | `TYPE_INT` | [L217](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L217) |
| `FIND_COLOR` | `BBB` | `TYPE_INT` | [L219](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L219) |
| `FIND_COLOR` | `IBBB` | `TYPE_INT` | [L220](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L220) |
| `FIND_COLOR` | `IIBBB` | `TYPE_INT` | [L221](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L221) |
| `GET_RGB` | `IIIPPP` | `TYPE_INT` | [L223](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L223) |
| `GET_RGBA` | `IIIPPPP` | `TYPE_INT` | [L224](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L224) |
| `GET_RGB` | `IPPPI` | `TYPE_INT` | [L225](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L225) |
| `GET_RGBA` | `IPPPPI` | `TYPE_INT` | [L226](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L226) |
| `GET_RGB` | `IPPP` | `TYPE_INT` | [L227](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L227) |
| `GET_RGBA` | `IPPPP` | `TYPE_INT` | [L228](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L228) |
| `NEW_FPG` | `` | `TYPE_INT` | [L231](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L231) |
| `LOAD_FPG` | `S` | `TYPE_INT` | [L232](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L232) |
| `LOAD_FPG` | `SP` | `TYPE_INT` | [L233](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L233) |
| `SAVE_FPG` | `IS` | `TYPE_INT` | [L234](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L234) |
| `UNLOAD_FPG` | `I` | `TYPE_INT` | [L235](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L235) |
| `GET_POINT` | `IIIPP` | `TYPE_INT` | [L238](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L238) |
| `SET_POINT` | `IIIII` | `TYPE_INT` | [L239](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L239) |
| `SET_CENTER` | `IIII` | `TYPE_INT` | [L240](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L240) |
| `NEW_FNT` | `I` | `TYPE_INT` | [L243](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L243) |
| `NEW_FNT` | `II` | `TYPE_INT` | [L244](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L244) |
| `NEW_FNT` | `IIIIIIII` | `TYPE_INT` | [L245](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L245) |
| `LOAD_FNT` | `S` | `TYPE_INT` | [L246](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L246) |
| `LOAD_FNT` | `SP` | `TYPE_INT` | [L247](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L247) |
| `UNLOAD_FNT` | `I` | `TYPE_INT` | [L248](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L248) |
| `SAVE_FNT` | `IS` | `TYPE_INT` | [L249](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L249) |
| `LOAD_BDF` | `S` | `TYPE_INT` | [L250](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L250) |
| `LOAD_BDF` | `SP` | `TYPE_INT` | [L251](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L251) |
| `GET_GLYPH` | `II` | `TYPE_INT` | [L252](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L252) |
| `SET_GLYPH` | `IIII` | `TYPE_INT` | [L253](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L253) |
| `LOAD_PNG` | `S` | `TYPE_INT` | [L256](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L256) |
| `LOAD_PNG` | `SP` | `TYPE_INT` | [L257](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L257) |
| `LOAD_PCX` | `S` | `TYPE_INT` | [L258](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L258) |
| `LOAD_PCX` | `SP` | `TYPE_INT` | [L259](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L259) |
| `SAVE_PNG` | `IIS` | `TYPE_INT` | [L262](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/mod_map_exports.h#L262) |

## bennugd1 / mod_math

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `ABS` | `F` | `TYPE_FLOAT` | [L55](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_math/mod_math_exports.h#L55) |
| `POW` | `FF` | `TYPE_FLOAT` | [L56](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_math/mod_math_exports.h#L56) |
| `SQRT` | `F` | `TYPE_FLOAT` | [L57](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_math/mod_math_exports.h#L57) |
| `COS` | `F` | `TYPE_FLOAT` | [L59](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_math/mod_math_exports.h#L59) |
| `SIN` | `F` | `TYPE_FLOAT` | [L60](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_math/mod_math_exports.h#L60) |
| `TAN` | `F` | `TYPE_FLOAT` | [L61](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_math/mod_math_exports.h#L61) |
| `ACOS` | `F` | `TYPE_FLOAT` | [L62](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_math/mod_math_exports.h#L62) |
| `ASIN` | `F` | `TYPE_FLOAT` | [L63](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_math/mod_math_exports.h#L63) |
| `ATAN` | `F` | `TYPE_FLOAT` | [L64](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_math/mod_math_exports.h#L64) |
| `ATAN2` | `FF` | `TYPE_FLOAT` | [L65](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_math/mod_math_exports.h#L65) |
| `ISINF` | `F` | `TYPE_INT` | [L67](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_math/mod_math_exports.h#L67) |
| `ISNAN` | `F` | `TYPE_INT` | [L68](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_math/mod_math_exports.h#L68) |
| `FINITE` | `F` | `TYPE_INT` | [L69](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_math/mod_math_exports.h#L69) |
| `FGET_ANGLE` | `IIII` | `TYPE_INT` | [L71](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_math/mod_math_exports.h#L71) |
| `FGET_DIST` | `IIII` | `TYPE_INT` | [L72](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_math/mod_math_exports.h#L72) |
| `NEAR_ANGLE` | `III` | `TYPE_INT` | [L73](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_math/mod_math_exports.h#L73) |
| `GET_DISTX` | `II` | `TYPE_INT` | [L74](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_math/mod_math_exports.h#L74) |
| `GET_DISTY` | `II` | `TYPE_INT` | [L75](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_math/mod_math_exports.h#L75) |

## bennugd1 / mod_mathi

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `ABS` | `F` | `TYPE_FLOAT` | [L53](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mathi/mod_mathi_exports.h#L53) |
| `POW` | `FF` | `TYPE_FLOAT` | [L54](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mathi/mod_mathi_exports.h#L54) |
| `SQRT` | `F` | `TYPE_FLOAT` | [L55](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mathi/mod_mathi_exports.h#L55) |
| `COS` | `I` | `TYPE_FLOAT` | [L57](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mathi/mod_mathi_exports.h#L57) |
| `SIN` | `I` | `TYPE_FLOAT` | [L58](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mathi/mod_mathi_exports.h#L58) |
| `TAN` | `I` | `TYPE_FLOAT` | [L59](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mathi/mod_mathi_exports.h#L59) |
| `ACOS` | `I` | `TYPE_FLOAT` | [L60](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mathi/mod_mathi_exports.h#L60) |
| `ASIN` | `I` | `TYPE_FLOAT` | [L61](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mathi/mod_mathi_exports.h#L61) |
| `ATAN` | `I` | `TYPE_FLOAT` | [L62](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mathi/mod_mathi_exports.h#L62) |
| `ATAN2` | `II` | `TYPE_FLOAT` | [L63](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mathi/mod_mathi_exports.h#L63) |
| `ISINF` | `F` | `TYPE_INT` | [L65](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mathi/mod_mathi_exports.h#L65) |
| `ISNAN` | `F` | `TYPE_INT` | [L66](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mathi/mod_mathi_exports.h#L66) |
| `FINITE` | `F` | `TYPE_INT` | [L67](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mathi/mod_mathi_exports.h#L67) |
| `FGET_ANGLE` | `IIII` | `TYPE_INT` | [L69](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mathi/mod_mathi_exports.h#L69) |
| `FGET_DIST` | `IIII` | `TYPE_INT` | [L70](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mathi/mod_mathi_exports.h#L70) |
| `NEAR_ANGLE` | `III` | `TYPE_INT` | [L71](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mathi/mod_mathi_exports.h#L71) |
| `GET_DISTX` | `II` | `TYPE_INT` | [L72](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mathi/mod_mathi_exports.h#L72) |
| `GET_DISTY` | `II` | `TYPE_INT` | [L73](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mathi/mod_mathi_exports.h#L73) |

## bennugd1 / mod_mem

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `MEM_CALLOC` | `II` | `TYPE_POINTER` | [L41](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mem/mod_mem_exports.h#L41) |
| `MEM_ALLOC` | `I` | `TYPE_POINTER` | [L42](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mem/mod_mem_exports.h#L42) |
| `MEM_FREE` | `P` | `TYPE_INT` | [L43](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mem/mod_mem_exports.h#L43) |
| `MEM_REALLOC` | `PI` | `TYPE_POINTER` | [L44](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mem/mod_mem_exports.h#L44) |
| `MEM_CMP` | `PPI` | `TYPE_INT` | [L45](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mem/mod_mem_exports.h#L45) |
| `MEM_SET` | `PBI` | `TYPE_INT` | [L46](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mem/mod_mem_exports.h#L46) |
| `MEM_SETW` | `PWI` | `TYPE_INT` | [L47](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mem/mod_mem_exports.h#L47) |
| `MEM_SETI` | `PII` | `TYPE_INT` | [L48](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mem/mod_mem_exports.h#L48) |
| `MEM_COPY` | `PPI` | `TYPE_INT` | [L49](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mem/mod_mem_exports.h#L49) |
| `MEM_MOVE` | `PPI` | `TYPE_INT` | [L50](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mem/mod_mem_exports.h#L50) |
| `MEM_AVAILABLE` | `` | `TYPE_INT` | [L51](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mem/mod_mem_exports.h#L51) |
| `MEM_TOTAL` | `` | `TYPE_INT` | [L52](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mem/mod_mem_exports.h#L52) |
| `CALLOC` | `II` | `TYPE_POINTER` | [L54](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mem/mod_mem_exports.h#L54) |
| `ALLOC` | `I` | `TYPE_POINTER` | [L55](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mem/mod_mem_exports.h#L55) |
| `FREE` | `P` | `TYPE_INT` | [L56](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mem/mod_mem_exports.h#L56) |
| `REALLOC` | `PI` | `TYPE_POINTER` | [L57](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mem/mod_mem_exports.h#L57) |
| `MEMCMP` | `PPI` | `TYPE_INT` | [L58](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mem/mod_mem_exports.h#L58) |
| `MEMSET` | `PBI` | `TYPE_INT` | [L59](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mem/mod_mem_exports.h#L59) |
| `MEMSETW` | `PWI` | `TYPE_INT` | [L60](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mem/mod_mem_exports.h#L60) |
| `MEMSETI` | `PII` | `TYPE_INT` | [L61](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mem/mod_mem_exports.h#L61) |
| `MEMCOPY` | `PPI` | `TYPE_INT` | [L62](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mem/mod_mem_exports.h#L62) |
| `MEMMOVE` | `PPI` | `TYPE_INT` | [L63](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mem/mod_mem_exports.h#L63) |
| `MEMORY_FREE` | `` | `TYPE_INT` | [L64](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mem/mod_mem_exports.h#L64) |
| `MEMORY_TOTAL` | `` | `TYPE_INT` | [L65](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_mem/mod_mem_exports.h#L65) |

## bennugd1 / mod_path

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `PATH_FIND` | `IIIIIII` | `TYPE_INT` | [L57](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_path/mod_path_exports.h#L57) |
| `PATH_GETXY` | `PP` | `TYPE_INT` | [L58](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_path/mod_path_exports.h#L58) |
| `PATH_WALL` | `I` | `TYPE_INT` | [L59](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_path/mod_path_exports.h#L59) |

## bennugd1 / mod_proc

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `GET_ID` | `I` | `TYPE_INT` | [L96](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_proc/mod_proc_exports.h#L96) |
| `GET_STATUS` | `I` | `TYPE_INT` | [L97](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_proc/mod_proc_exports.h#L97) |
| `SIGNAL` | `II` | `TYPE_INT` | [L98](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_proc/mod_proc_exports.h#L98) |
| `SIGNAL_ACTION` | `II` | `TYPE_INT` | [L99](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_proc/mod_proc_exports.h#L99) |
| `SIGNAL_ACTION` | `III` | `TYPE_INT` | [L100](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_proc/mod_proc_exports.h#L100) |
| `LET_ME_ALONE` | `` | `TYPE_INT` | [L101](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_proc/mod_proc_exports.h#L101) |
| `EXIT` | `SI` | `TYPE_INT` | [L102](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_proc/mod_proc_exports.h#L102) |
| `EXIT` | `S` | `TYPE_INT` | [L103](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_proc/mod_proc_exports.h#L103) |
| `EXIT` | `` | `TYPE_INT` | [L104](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_proc/mod_proc_exports.h#L104) |
| `EXISTS` | `I` | `TYPE_INT` | [L105](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_proc/mod_proc_exports.h#L105) |

## bennugd1 / mod_rand

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `RAND_SEED` | `I` | `TYPE_INT` | [L41](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_rand/mod_rand_exports.h#L41) |
| `RAND` | `II` | `TYPE_INT` | [L42](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_rand/mod_rand_exports.h#L42) |

## bennugd1 / mod_regex

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `REGEX` | `SS` | `TYPE_INT` | [L54](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_regex/mod_regex_exports.h#L54) |
| `REGEX_REPLACE` | `SSS` | `TYPE_STRING` | [L55](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_regex/mod_regex_exports.h#L55) |
| `SPLIT` | `SSPI` | `TYPE_INT` | [L56](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_regex/mod_regex_exports.h#L56) |
| `JOIN` | `SPI` | `TYPE_STRING` | [L57](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_regex/mod_regex_exports.h#L57) |

## bennugd1 / mod_say

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `SAY` | `S` | `TYPE_UNDEFINED` | [L41](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_say/mod_say_exports.h#L41) |
| `SAY_FAST` | `S` | `TYPE_UNDEFINED` | [L42](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_say/mod_say_exports.h#L42) |

## bennugd1 / mod_screen

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `REGION_DEFINE` | `IIIII` | `TYPE_INT` | [L41](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_screen/mod_screen_exports.h#L41) |
| `REGION_OUT` | `II` | `TYPE_INT` | [L42](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_screen/mod_screen_exports.h#L42) |
| `PUT` | `IIII` | `TYPE_INT` | [L45](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_screen/mod_screen_exports.h#L45) |
| `XPUT` | `IIIIIIII` | `TYPE_INT` | [L46](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_screen/mod_screen_exports.h#L46) |
| `SCREEN_PUT` | `II` | `TYPE_INT` | [L47](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_screen/mod_screen_exports.h#L47) |
| `SCREEN_CLEAR` | `` | `TYPE_INT` | [L48](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_screen/mod_screen_exports.h#L48) |
| `SCREEN_GET` | `` | `TYPE_INT` | [L51](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_screen/mod_screen_exports.h#L51) |
| `DEFINE_REGION` | `IIIII` | `TYPE_INT` | [L56](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_screen/mod_screen_exports.h#L56) |
| `OUT_REGION` | `II` | `TYPE_INT` | [L57](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_screen/mod_screen_exports.h#L57) |
| `PUT_SCREEN` | `II` | `TYPE_INT` | [L60](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_screen/mod_screen_exports.h#L60) |
| `CLEAR_SCREEN` | `` | `TYPE_INT` | [L61](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_screen/mod_screen_exports.h#L61) |
| `GET_SCREEN` | `` | `TYPE_INT` | [L64](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_screen/mod_screen_exports.h#L64) |

## bennugd1 / mod_scroll

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `SCROLL_START` | `IIIIIIII` | `TYPE_INT` | [L40](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_scroll/mod_scroll_exports.h#L40) |
| `SCROLL_START` | `IIIIII` | `TYPE_INT` | [L41](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_scroll/mod_scroll_exports.h#L41) |
| `SCROLL_STOP` | `I` | `TYPE_INT` | [L42](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_scroll/mod_scroll_exports.h#L42) |
| `SCROLL_MOVE` | `I` | `TYPE_INT` | [L43](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_scroll/mod_scroll_exports.h#L43) |
| `START_SCROLL` | `IIIIIIII` | `TYPE_INT` | [L45](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_scroll/mod_scroll_exports.h#L45) |
| `START_SCROLL` | `IIIIII` | `TYPE_INT` | [L46](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_scroll/mod_scroll_exports.h#L46) |
| `STOP_SCROLL` | `I` | `TYPE_INT` | [L47](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_scroll/mod_scroll_exports.h#L47) |
| `MOVE_SCROLL` | `I` | `TYPE_INT` | [L48](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_scroll/mod_scroll_exports.h#L48) |

## bennugd1 / mod_sort

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `QUICKSORT` | `PIIIBB` | `TYPE_INT` | [L41](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sort/mod_sort_exports.h#L41) |
| `KSORT` | `V++V++` | `TYPE_INT` | [L42](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sort/mod_sort_exports.h#L42) |
| `KSORT` | `V++V++I` | `TYPE_INT` | [L43](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sort/mod_sort_exports.h#L43) |
| `SORT` | `V++I` | `TYPE_INT` | [L44](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sort/mod_sort_exports.h#L44) |
| `SORT` | `V++` | `TYPE_INT` | [L45](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sort/mod_sort_exports.h#L45) |

## bennugd1 / mod_sound

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `SOUND_INIT` | `` | `TYPE_INT` | [L65](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sound/mod_sound_exports.h#L65) |
| `SOUND_CLOSE` | `` | `TYPE_INT` | [L66](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sound/mod_sound_exports.h#L66) |
| `LOAD_SONG` | `S` | `TYPE_INT` | [L68](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sound/mod_sound_exports.h#L68) |
| `LOAD_SONG` | `SP` | `TYPE_INT` | [L69](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sound/mod_sound_exports.h#L69) |
| `UNLOAD_SONG` | `I` | `TYPE_INT` | [L70](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sound/mod_sound_exports.h#L70) |
| `UNLOAD_SONG` | `P` | `TYPE_INT` | [L71](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sound/mod_sound_exports.h#L71) |
| `PLAY_SONG` | `II` | `TYPE_INT` | [L73](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sound/mod_sound_exports.h#L73) |
| `STOP_SONG` | `` | `TYPE_INT` | [L74](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sound/mod_sound_exports.h#L74) |
| `PAUSE_SONG` | `` | `TYPE_INT` | [L75](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sound/mod_sound_exports.h#L75) |
| `RESUME_SONG` | `` | `TYPE_INT` | [L76](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sound/mod_sound_exports.h#L76) |
| `SET_SONG_VOLUME` | `I` | `TYPE_INT` | [L78](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sound/mod_sound_exports.h#L78) |
| `IS_PLAYING_SONG` | `` | `TYPE_INT` | [L80](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sound/mod_sound_exports.h#L80) |
| `LOAD_WAV` | `S` | `TYPE_INT` | [L82](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sound/mod_sound_exports.h#L82) |
| `LOAD_WAV` | `SP` | `TYPE_INT` | [L83](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sound/mod_sound_exports.h#L83) |
| `UNLOAD_WAV` | `I` | `TYPE_INT` | [L84](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sound/mod_sound_exports.h#L84) |
| `UNLOAD_WAV` | `P` | `TYPE_INT` | [L85](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sound/mod_sound_exports.h#L85) |
| `PLAY_WAV` | `II` | `TYPE_INT` | [L87](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sound/mod_sound_exports.h#L87) |
| `PLAY_WAV` | `III` | `TYPE_INT` | [L88](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sound/mod_sound_exports.h#L88) |
| `STOP_WAV` | `I` | `TYPE_INT` | [L89](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sound/mod_sound_exports.h#L89) |
| `PAUSE_WAV` | `I` | `TYPE_INT` | [L90](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sound/mod_sound_exports.h#L90) |
| `RESUME_WAV` | `I` | `TYPE_INT` | [L91](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sound/mod_sound_exports.h#L91) |
| `IS_PLAYING_WAV` | `I` | `TYPE_INT` | [L93](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sound/mod_sound_exports.h#L93) |
| `FADE_MUSIC_IN` | `III` | `TYPE_INT` | [L95](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sound/mod_sound_exports.h#L95) |
| `FADE_MUSIC_OFF` | `I` | `TYPE_INT` | [L96](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sound/mod_sound_exports.h#L96) |
| `SET_WAV_VOLUME` | `II` | `TYPE_INT` | [L98](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sound/mod_sound_exports.h#L98) |
| `SET_CHANNEL_VOLUME` | `II` | `TYPE_INT` | [L99](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sound/mod_sound_exports.h#L99) |
| `RESERVE_CHANNELS` | `I` | `TYPE_INT` | [L101](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sound/mod_sound_exports.h#L101) |
| `SET_PANNING` | `III` | `TYPE_INT` | [L103](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sound/mod_sound_exports.h#L103) |
| `SET_POSITION` | `III` | `TYPE_INT` | [L104](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sound/mod_sound_exports.h#L104) |
| `SET_DISTANCE` | `II` | `TYPE_INT` | [L105](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sound/mod_sound_exports.h#L105) |
| `REVERSE_STEREO` | `II` | `TYPE_INT` | [L107](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sound/mod_sound_exports.h#L107) |
| `SET_MUSIC_POSITION` | `F` | `TYPE_INT` | [L109](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sound/mod_sound_exports.h#L109) |

## bennugd1 / mod_string

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `STRLEN` | `S` | `TYPE_INT` | [L41](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_string/mod_string_exports.h#L41) |
| `LEN` | `S` | `TYPE_INT` | [L42](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_string/mod_string_exports.h#L42) |
| `UCASE` | `S` | `TYPE_STRING` | [L43](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_string/mod_string_exports.h#L43) |
| `LCASE` | `S` | `TYPE_STRING` | [L44](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_string/mod_string_exports.h#L44) |
| `STRCASECMP` | `SS` | `TYPE_INT` | [L45](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_string/mod_string_exports.h#L45) |
| `SUBSTR` | `SII` | `TYPE_STRING` | [L46](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_string/mod_string_exports.h#L46) |
| `SUBSTR` | `SI` | `TYPE_STRING` | [L47](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_string/mod_string_exports.h#L47) |
| `FIND` | `SS` | `TYPE_INT` | [L48](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_string/mod_string_exports.h#L48) |
| `FIND` | `SSI` | `TYPE_INT` | [L49](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_string/mod_string_exports.h#L49) |
| `LPAD` | `SI` | `TYPE_STRING` | [L50](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_string/mod_string_exports.h#L50) |
| `RPAD` | `SI` | `TYPE_STRING` | [L51](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_string/mod_string_exports.h#L51) |
| `ITOA` | `I` | `TYPE_STRING` | [L52](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_string/mod_string_exports.h#L52) |
| `FTOA` | `F` | `TYPE_STRING` | [L53](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_string/mod_string_exports.h#L53) |
| `ATOI` | `S` | `TYPE_INT` | [L54](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_string/mod_string_exports.h#L54) |
| `ATOF` | `S` | `TYPE_FLOAT` | [L55](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_string/mod_string_exports.h#L55) |
| `ASC` | `S` | `TYPE_BYTE` | [L56](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_string/mod_string_exports.h#L56) |
| `CHR` | `I` | `TYPE_STRING` | [L57](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_string/mod_string_exports.h#L57) |
| `TRIM` | `S` | `TYPE_STRING` | [L58](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_string/mod_string_exports.h#L58) |
| `STRREV` | `S` | `TYPE_STRING` | [L59](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_string/mod_string_exports.h#L59) |
| `FORMAT` | `I` | `TYPE_STRING` | [L60](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_string/mod_string_exports.h#L60) |
| `FORMAT` | `F` | `TYPE_STRING` | [L61](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_string/mod_string_exports.h#L61) |
| `FORMAT` | `FI` | `TYPE_STRING` | [L62](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_string/mod_string_exports.h#L62) |
| `STRING_BUFFER` | `S` | `TYPE_POINTER` | [L64](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_string/mod_string_exports.h#L64) |
| `STRING_ALLOC` | `` | `TYPE_POINTER` | [L65](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_string/mod_string_exports.h#L65) |
| `STRING_ALLOC` | `S` | `TYPE_POINTER` | [L66](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_string/mod_string_exports.h#L66) |
| `STRING_RELEASE` | `P` | `TYPE_UNDEFINED` | [L67](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_string/mod_string_exports.h#L67) |
| `STRING_DUMP` | `` | `TYPE_UNDEFINED` | [L69](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_string/mod_string_exports.h#L69) |

## bennugd1 / mod_sys

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `GETENV` | `S` | `TYPE_STRING` | [L57](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sys/mod_sys_exports.h#L57) |
| `EXEC` | `ISIP` | `TYPE_INT` | [L58](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_sys/mod_sys_exports.h#L58) |

## bennugd1 / mod_text

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `WRITE` | `IIIIS` | `TYPE_INT` | [L71](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_text/mod_text_exports.h#L71) |
| `WRITE` | `IIIIIS` | `TYPE_INT` | [L72](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_text/mod_text_exports.h#L72) |
| `WRITE_INT` | `IIIIP` | `TYPE_INT` | [L73](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_text/mod_text_exports.h#L73) |
| `WRITE_INT` | `IIIIIP` | `TYPE_INT` | [L74](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_text/mod_text_exports.h#L74) |
| `MOVE_TEXT` | `III` | `TYPE_INT` | [L75](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_text/mod_text_exports.h#L75) |
| `MOVE_TEXT` | `IIII` | `TYPE_INT` | [L76](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_text/mod_text_exports.h#L76) |
| `DELETE_TEXT` | `I` | `TYPE_INT` | [L77](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_text/mod_text_exports.h#L77) |
| `WRITE_IN_MAP` | `ISI` | `TYPE_INT` | [L78](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_text/mod_text_exports.h#L78) |
| `TEXT_WIDTH` | `IS` | `TYPE_INT` | [L79](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_text/mod_text_exports.h#L79) |
| `TEXT_HEIGHT` | `IS` | `TYPE_INT` | [L80](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_text/mod_text_exports.h#L80) |
| `GET_TEXT_COLOR` | `` | `TYPE_INT` | [L81](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_text/mod_text_exports.h#L81) |
| `GET_TEXT_COLOR` | `I` | `TYPE_INT` | [L82](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_text/mod_text_exports.h#L82) |
| `SET_TEXT_COLOR` | `I` | `TYPE_INT` | [L83](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_text/mod_text_exports.h#L83) |
| `SET_TEXT_COLOR` | `II` | `TYPE_INT` | [L84](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_text/mod_text_exports.h#L84) |
| `WRITE_VAR` | `IIIIV++` | `TYPE_INT` | [L87](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_text/mod_text_exports.h#L87) |
| `WRITE_VAR` | `IIIIIV++` | `TYPE_INT` | [L88](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_text/mod_text_exports.h#L88) |
| `WRITE_FLOAT` | `IIIIP` | `TYPE_INT` | [L89](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_text/mod_text_exports.h#L89) |
| `WRITE_FLOAT` | `IIIIIP` | `TYPE_INT` | [L90](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_text/mod_text_exports.h#L90) |
| `WRITE_STRING` | `IIIIP` | `TYPE_INT` | [L91](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_text/mod_text_exports.h#L91) |
| `WRITE_STRING` | `IIIIIP` | `TYPE_INT` | [L92](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_text/mod_text_exports.h#L92) |

## bennugd1 / mod_time

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `GET_TIMER` | `` | `TYPE_INT` | [L42](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_time/mod_time_exports.h#L42) |
| `TIME` | `` | `TYPE_INT` | [L43](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_time/mod_time_exports.h#L43) |
| `FTIME` | `SI` | `TYPE_STRING` | [L44](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_time/mod_time_exports.h#L44) |

## bennugd1 / mod_video

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `SET_MODE` | `I` | `TYPE_INT` | [L44](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_video/mod_video_exports.h#L44) |
| `SET_MODE` | `II` | `TYPE_INT` | [L45](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_video/mod_video_exports.h#L45) |
| `SET_MODE` | `III` | `TYPE_INT` | [L46](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_video/mod_video_exports.h#L46) |
| `SET_MODE` | `IIII` | `TYPE_INT` | [L47](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_video/mod_video_exports.h#L47) |
| `SET_FPS` | `II` | `TYPE_INT` | [L48](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_video/mod_video_exports.h#L48) |
| `GET_MODES` | `II` | `TYPE_POINTER` | [L50](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_video/mod_video_exports.h#L50) |
| `MODE_IS_OK` | `IIII` | `TYPE_INT` | [L51](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_video/mod_video_exports.h#L51) |

## bennugd1 / mod_wm

| Nombre | Firma original | Retorno | Fuente |
| --- | --- | --- | --- |
| `SET_TITLE` | `S` | `TYPE_INT` | [L41](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_wm/mod_wm_exports.h#L41) |
| `SET_ICON` | `II` | `TYPE_INT` | [L42](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_wm/mod_wm_exports.h#L42) |
| `MINIMIZE` | `` | `TYPE_INT` | [L43](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_wm/mod_wm_exports.h#L43) |
| `MOVE_WINDOW` | `II` | `TYPE_INT` | [L44](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_wm/mod_wm_exports.h#L44) |
| `SET_WINDOW_POS` | `II` | `TYPE_INT` | [L45](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_wm/mod_wm_exports.h#L45) |
| `GET_WINDOW_POS` | `PP` | `TYPE_INT` | [L46](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_wm/mod_wm_exports.h#L46) |
| `GET_WINDOW_SIZE` | `PPPP` | `TYPE_INT` | [L47](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_wm/mod_wm_exports.h#L47) |
| `GET_DESKTOP_SIZE` | `PP` | `TYPE_INT` | [L48](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_wm/mod_wm_exports.h#L48) |
