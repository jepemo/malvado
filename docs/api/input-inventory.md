# Input API baseline for alpha.6

This inventory records the delivered input API. It is an input to the complete
2.0 inventory, not a selection of new input features. See the [input guide](lua-processes.md)
for names, return values, errors, thread rules, and headless behavior.

| Area | C | Lua | Examples and automated evidence |
| --- | --- | --- | --- |
| Keyboard | `mv_key_from_name`, `mv_key_down`, `mv_window_escape_down`, `mv_window_space_down` | `mv.key`, `mv.key_down`, `mv.input.key_down`, `KEY_*` constants | [timing game](../../examples/c/timing-game/README.md), `input`, `keys`, `visual_c_keys`, `visual_lua_keys` |
| Mouse | `mv_mouse_button_from_name`, `mv_mouse_position`, `mv_mouse_down` | `mv.mouse_position`, `mv.mouse_button` and their `mv.input` aliases | [demo](../../examples/demo/main.lua), `input`, `keys`, `visual_c_mouse`, `visual_lua_mouse` |
| Controller | `mv_pad_connected`, `mv_pad_button_down`, `mv_pad_axis`, `mv_pad_button_from_name`, `mv_pad_axis_from_name` | `mv.pad_connected`, `mv.pad_button`, `mv.pad_axis` and their `mv.input` aliases | [controller monitor](../../examples/controllers/README.md), `input`, `lua_controllers` |
| Text input | `mv_text_input_next` | `mv.text_input_next` and `mv.input.text_input_next` | [demo](../../examples/demo/main.lua), `input`, `keys`, `visual_c_text_input`, `visual_lua_text_input` |
| UTF-8 conversion | `mv_utf8_encode`, `mv_utf8_decode` | No separate Malvado Lua API; text input returns UTF-8 strings | [UTF-8 C example](../../examples/c/utf8/README.md), `input`, `c_utf8_example`, `visual_c_text_input`, `visual_lua_text_input` |

The keyboard, mouse, controller, and text APIs use the platform adapter; they do
not require a second scheduler. Input is updated when the window processes
events. The C adapter returns zero or false for unavailable input. Lua rejects
invalid names and slots, and returns neutral input in headless mode. The C UTF-8
helpers do not need a window. The input guide defines queue and ownership limits.

The keyboard, mouse, and text event tests require a supported Linux display and
`xdotool`; they are not registered in every build. The controller tests check
names, validation, and disconnected behavior. Physical-controller mappings and
device checks remain in beta; the automated tests do not certify hardware.
This inventory records existing behavior and does not decide additional keys,
touch, rumble, or event APIs for 2.0.
