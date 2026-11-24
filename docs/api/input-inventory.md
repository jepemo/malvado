# Input API inventory

This inventory records the C and Lua input APIs. See the [input guide](lua-processes.md)
for names, return values, errors, thread rules, and headless behavior.

| Area | C | Lua | Examples and automated evidence |
| --- | --- | --- | --- |
| Keyboard | `mv_key_from_name`, `mv_key_down`, `mv_key_pressed`, `mv_key_released`, `mv_input_tick`, `mv_window_escape_down`, `mv_window_space_down` | `mv.key`, `mv.key_pressed`, `mv.key_released` and their `mv.input` aliases, `KEY_*` constants | [timing game](../../examples/c/timing-game/README.md), `input`, `keys`, `visual_c_keys`, `visual_lua_keys` |
| Mouse | `mv_mouse_button_from_name`, `mv_mouse_position`, `mv_mouse_down`, `mv_mouse_pressed`, `mv_mouse_released` | `mv.mouse_position`, `mv.mouse_button`, `mv.mouse_pressed`, `mv.mouse_released` and their `mv.input` aliases, `MOUSE_*` constants | [demo](../../examples/demo/main.lua), `input`, `keys`, `visual_c_mouse`, `visual_lua_mouse` |
| Controller | `mv_pad_connected`, `mv_pad_button_down`, `mv_pad_button_pressed`, `mv_pad_button_released`, `mv_pad_axis`, `mv_pad_button_from_name`, `mv_pad_axis_from_name` | `mv.pad_connected`, `mv.pad_button`, `mv.pad_pressed`, `mv.pad_released`, `mv.pad_axis` and their `mv.input` aliases, `PAD_*` constants | [controller monitor](../../examples/controllers/README.md), `input`, `lua_controllers` |
| On-screen keyboard | `mv_text_input_set_keyboard` | `mv.text_input_set_keyboard` and `mv.input.text_input_set_keyboard` | [keyboard](keyboard.md#android-on-screen-keyboard), `input`, `keys` |
| Text input | `mv_text_input_next` | `mv.text_input_next` and `mv.input.text_input_next` | [demo](../../examples/demo/main.lua), `input`, `keys`, `visual_c_text_input`, `visual_lua_text_input` |
| UTF-8 conversion | `mv_utf8_encode`, `mv_utf8_decode` | No separate Malvado Lua API; text input returns UTF-8 strings | [UTF-8 C example](../../examples/c/utf8/README.md), `input`, `c_utf8_example`, `visual_c_text_input`, `visual_lua_text_input` |

Held input updates when the window processes events. The engine buffers
observed key, mouse, and controller transitions and commits them once per
logical tick with `mv_input_tick`. The Lua runner commits them automatically.
Held-state and transition queries are non-consuming. Text input consumes one
scalar from its shared queue. See the [logical tick contract](keyboard.md).

The C adapter returns zero for unavailable input. Name conversion works without
a window and leaves the output unchanged on failure. Lua rejects invalid names
and slots and gives neutral input in headless mode. Rust uses typed controls and
`char`. See the [Rust input guide](rust-input.md).

The keyboard, mouse, and text event tests require a supported Linux display and
`xdotool`; they are not registered in every build. The controller tests check
validation, independent slots, tick timing, and connection baselines. Controller
mappings and hotplug behavior depend on the backend and device. The bundled
desktop GLFW backend reports slot changes even when a slot is reused within a
single poll. Other backends use the connection state exposed by raylib.

## C input types and constants

| C type | C constants | Lua relationship |
| --- | --- | --- |
| `MvKey` | `MV_KEY_0`, `MV_KEY_1`, `MV_KEY_2`, `MV_KEY_3`, `MV_KEY_4`, `MV_KEY_5`, `MV_KEY_6`, `MV_KEY_7`, `MV_KEY_8`, `MV_KEY_9`, `MV_KEY_A`, `MV_KEY_B`, `MV_KEY_C`, `MV_KEY_D`, `MV_KEY_E`, `MV_KEY_F`, `MV_KEY_G`, `MV_KEY_H`, `MV_KEY_I`, `MV_KEY_J`, `MV_KEY_K`, `MV_KEY_L`, `MV_KEY_M`, `MV_KEY_N`, `MV_KEY_O`, `MV_KEY_P`, `MV_KEY_Q`, `MV_KEY_R`, `MV_KEY_S`, `MV_KEY_T`, `MV_KEY_U`, `MV_KEY_V`, `MV_KEY_W`, `MV_KEY_X`, `MV_KEY_Y`, `MV_KEY_Z`, `MV_KEY_ESCAPE`, `MV_KEY_SPACE`, `MV_KEY_LEFT`, `MV_KEY_RIGHT`, `MV_KEY_UP`, `MV_KEY_DOWN`, `MV_KEY_ENTER`, `MV_KEY_TAB`, `MV_KEY_BACKSPACE` | Lua uses case-sensitive names and `KEY_*` string constants. |
| `MvMouseButton` | `MV_MOUSE_LEFT`, `MV_MOUSE_MIDDLE`, `MV_MOUSE_RIGHT` | Lua uses `MOUSE_LEFT`, `MOUSE_MIDDLE`, and `MOUSE_RIGHT` string constants. |
| `MvPadButton` | `MV_PAD_A`, `MV_PAD_B`, `MV_PAD_X`, `MV_PAD_Y`, `MV_PAD_UP`, `MV_PAD_DOWN`, `MV_PAD_LEFT`, `MV_PAD_RIGHT`, `MV_PAD_LB`, `MV_PAD_RB`, `MV_PAD_BACK`, `MV_PAD_START` | Lua uses the matching `PAD_*` button string constants. |
| `MvPadAxis` | `MV_PAD_LEFT_X`, `MV_PAD_LEFT_Y`, `MV_PAD_RIGHT_X`, `MV_PAD_RIGHT_Y`, `MV_PAD_LEFT_TRIGGER`, `MV_PAD_RIGHT_TRIGGER` | Lua uses the matching `PAD_*` axis string constants. |

`input_api_map` checks the documented function, type, and constant names against
`platform.h`. It uses the input prefixes and the two window-key helpers listed
in its source; it does not detect a new API with an unrelated name. It checks
names only, not values or semantics. `input`, `keys`, and `lua_controllers`
check conversions and headless behavior. The [global table](lua-globals.md)
and `lua_global_inventory` cover Lua exports and aliases.

## Android primary contact

The current Android raylib backend maps the first touch contact to the existing
left-mouse state and logical window coordinates. No separate Malvado touch API
is added. The emulator check holds one contact, moves it horizontally, observes
release and then exits through Back. The fixture checks C, Lua, and Rust.
The Android event callback preserves both edges from a short tap before the
next tick. The native callback test checks this case and cancellation. See
[Android verification](../development/android.md).

Android pointer buttons become neutral when the Activity pauses, loses focus
or loses its native surface. Native cancel/up events also clear the primary
contact and record its release. A fresh down event is required after the
Activity regains focus. Position queries return logical window coordinates.
