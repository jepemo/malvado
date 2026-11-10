# Input API baseline for alpha.6

This inventory records the delivered input API. It is an input to the complete
2.0 inventory, not a selection of new input features. See the [input guide](lua-processes.md)
for names, return values, errors, thread rules, and headless behavior.

| Area | C | Lua | Examples and automated evidence |
| --- | --- | --- | --- |
| Keyboard | `mv_key_from_name`, `mv_key_down`, `mv_key_pressed`, `mv_key_released`, `mv_input_tick`, `mv_window_escape_down`, `mv_window_space_down` | `mv.key`, `mv.key_down`, `mv.key_pressed`, `mv.key_released`, `mv.input.key_down`, `mv.input.key_pressed`, `mv.input.key_released`, `KEY_*` constants | [timing game](../../examples/c/timing-game/README.md), `input`, `keys`, `visual_c_keys`, `visual_lua_keys` |
| Mouse | `mv_mouse_button_from_name`, `mv_mouse_position`, `mv_mouse_down` | `mv.mouse_position`, `mv.mouse_button` and their `mv.input` aliases | [demo](../../examples/demo/main.lua), `input`, `keys`, `visual_c_mouse`, `visual_lua_mouse` |
| Controller | `mv_pad_connected`, `mv_pad_button_down`, `mv_pad_axis`, `mv_pad_button_from_name`, `mv_pad_axis_from_name` | `mv.pad_connected`, `mv.pad_button`, `mv.pad_axis` and their `mv.input` aliases | [controller monitor](../../examples/controllers/README.md), `input`, `lua_controllers` |
| Text input | `mv_text_input_next` | `mv.text_input_next` and `mv.input.text_input_next` | [demo](../../examples/demo/main.lua), `input`, `keys`, `visual_c_text_input`, `visual_lua_text_input` |
| UTF-8 conversion | `mv_utf8_encode`, `mv_utf8_decode` | No separate Malvado Lua API; text input returns UTF-8 strings | [UTF-8 C example](../../examples/c/utf8/README.md), `input`, `c_utf8_example`, `visual_c_text_input`, `visual_lua_text_input` |

The keyboard, mouse, controller, and text APIs use the platform adapter; they do
not require a second scheduler. Held input is updated when the window processes
events. Keyboard edges are buffered at frame end and committed once per logical
tick with `mv_input_tick`; the Lua runner commits automatically. Queries are
non-consuming. See the [keyboard edge contract](keyboard.md). The C adapter returns zero or false for unavailable input. Lua rejects
invalid names and slots, and returns neutral input in headless mode. The C UTF-8
helpers do not need a window. The input guide defines queue and ownership limits.

The keyboard, mouse, and text event tests require a supported Linux display and
`xdotool`; they are not registered in every build. The controller tests check
names, validation, and disconnected behavior. Physical-controller mappings and
device checks remain in beta; the automated tests do not certify hardware.
This inventory records existing behavior and does not decide additional keys,
touch, rumble, or event APIs for 2.0.

## C input types and constants

| C type | C constants | Lua relationship |
| --- | --- | --- |
| `MvKey` | `MV_KEY_0`, `MV_KEY_1`, `MV_KEY_2`, `MV_KEY_3`, `MV_KEY_4`, `MV_KEY_5`, `MV_KEY_6`, `MV_KEY_7`, `MV_KEY_8`, `MV_KEY_9`, `MV_KEY_A`, `MV_KEY_B`, `MV_KEY_C`, `MV_KEY_D`, `MV_KEY_E`, `MV_KEY_F`, `MV_KEY_G`, `MV_KEY_H`, `MV_KEY_I`, `MV_KEY_J`, `MV_KEY_K`, `MV_KEY_L`, `MV_KEY_M`, `MV_KEY_N`, `MV_KEY_O`, `MV_KEY_P`, `MV_KEY_Q`, `MV_KEY_R`, `MV_KEY_S`, `MV_KEY_T`, `MV_KEY_U`, `MV_KEY_V`, `MV_KEY_W`, `MV_KEY_X`, `MV_KEY_Y`, `MV_KEY_Z`, `MV_KEY_ESCAPE`, `MV_KEY_SPACE`, `MV_KEY_LEFT`, `MV_KEY_RIGHT`, `MV_KEY_UP`, `MV_KEY_DOWN`, `MV_KEY_ENTER`, `MV_KEY_TAB`, `MV_KEY_BACKSPACE` | Lua uses case-sensitive names and `KEY_*` string constants. |
| `MvMouseButton` | `MV_MOUSE_LEFT`, `MV_MOUSE_MIDDLE`, `MV_MOUSE_RIGHT` | Lua uses `left`, `middle`, and `right` strings. |
| `MvPadButton` | `MV_PAD_A`, `MV_PAD_B`, `MV_PAD_X`, `MV_PAD_Y`, `MV_PAD_UP`, `MV_PAD_DOWN`, `MV_PAD_LEFT`, `MV_PAD_RIGHT`, `MV_PAD_LB`, `MV_PAD_RB`, `MV_PAD_BACK`, `MV_PAD_START` | Lua uses the button name strings in the input guide. |
| `MvPadAxis` | `MV_PAD_LEFT_X`, `MV_PAD_LEFT_Y`, `MV_PAD_RIGHT_X`, `MV_PAD_RIGHT_Y`, `MV_PAD_LEFT_TRIGGER`, `MV_PAD_RIGHT_TRIGGER` | Lua uses the axis name strings in the input guide. |

`input_api_map` checks the documented function, type, and constant names against
`platform.h`. It uses the input prefixes and the two window-key helpers listed
in its source; it does not detect a new API with an unrelated name. It checks
names only, not values or semantics. `input`, `keys`, and `lua_controllers`
check conversions and headless behavior. The [global table](lua-globals.md)
and `lua_global_inventory` cover Lua exports and aliases.
